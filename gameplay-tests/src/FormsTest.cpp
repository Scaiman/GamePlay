#include "FormsTest.h"

#if defined(ADD_TEST)
    ADD_TEST("Graphics", "Forms", FormsTest, 101);
#endif

// Input bit-flags (powers of 2)
#define KEY_A_MASK (1 << 0)
#define KEY_B_MASK (1 << 1)

#define BUTTON_A (_gamepad->isVirtual() ? 0 : 10)
#define BUTTON_B (_gamepad->isVirtual() ? 1 : 11)

const static unsigned int __formsCount = 5;

FormsTest::FormsTest()
    : _scene(NULL), _formNode(NULL), _formNodeParent(NULL), _formSelect(NULL), _activeForm(NULL), _gamepad(NULL), _keyFlags(0)
{
    const char* formFiles[] = 
    {
        "res/common/formBasicControls.form",
        "res/common/formScrolling.form",
        "res/common/formFlowLayout.form",
        "res/common/formVerticalLayout.form",
        "res/common/formZOrder.form",
    };

    _formFiles.assign(formFiles, formFiles + __formsCount);
}

FormsTest::~FormsTest()
{
}

FormsTest* FormsTest::create()
{
    return new FormsTest();
}

void FormsTest::finalize()
{
    SAFE_RELEASE(_scene);
    SAFE_RELEASE(_formSelect);
    for (unsigned int i = 0; i < _forms.size(); i++)
    {
        SAFE_RELEASE(_forms[i]);
    }
}

void printProperties(Properties* properties, unsigned int tabCount)
{
    // Print the name and ID of the current namespace.
    const char* spacename = properties->getNamespace();
    const char* id = properties->getId();
    std::string tabs;
    for (unsigned int i = 0; i < tabCount; i++)
    {
        tabs.append("\t");
    }
    GP_WARN("\n%s%s %s\n%s{", tabs.c_str(), spacename, id, tabs.c_str());
 
    // Print all properties in this namespace.
    const char* name = properties->getNextProperty();
    const char* value = NULL;
    while (name != NULL)
    {
        value = properties->getString(name);
        GP_WARN("%s\t%s = %s", tabs.c_str(), name, value);
        name = properties->getNextProperty();
    }
 
    // Print the properties of every namespace within this one.
    Properties* space = properties->getNextNamespace();
    while (space != NULL)
    {
        printProperties(space, tabCount+1);
        space = properties->getNextNamespace();
    }

    GP_WARN("%s}", tabs.c_str());
}

void FormsTest::initialize()
{
    setMultiTouch(true);
    setVsync(false);

    _formSelect = Form::create("res/common/formSelect.form");
    
    RadioButton* form0Button = static_cast<RadioButton*>(_formSelect->getControl("form0"));
    form0Button->addListener(this, Control::Listener::CLICK);

    RadioButton* form1Button = static_cast<RadioButton*>(_formSelect->getControl("form1"));
    form1Button->addListener(this, Control::Listener::CLICK);

    RadioButton* form2Button = static_cast<RadioButton*>(_formSelect->getControl("form2"));
    form2Button->addListener(this, Control::Listener::CLICK);
    
    RadioButton* form3Button = static_cast<RadioButton*>(_formSelect->getControl("form3"));
    form3Button->addListener(this, Control::Listener::CLICK);

    RadioButton* form4Button = static_cast<RadioButton*>(_formSelect->getControl("form4"));
    form4Button->addListener(this, Control::Listener::CLICK);
    
    RadioButton* form5Button = static_cast<RadioButton*>(_formSelect->getControl("form5"));
    form5Button->addListener(this, Control::Listener::CLICK);
    
    for (unsigned int i = 0; i < _formFiles.size(); i++)
    {
        Form* form = Form::create(_formFiles[i]);
        form->setEnabled(false);
        _forms.push_back(form);
    }
    _formIndex = 0;

    // Create a form programmatically.
    createTestForm(_forms[0]->getTheme()->getStyle("buttonStyle"));

    Button* button = static_cast<Button*>(_forms[0]->getControl("testButton"));
    button->addListener(this, Control::Listener::CLICK);

    // Create a scene with a camera node.
    Camera* camera = Camera::createPerspective(45.0f, (float)getWidth() / (float)getHeight(), 0.25f, 100.0f);
    _scene = Scene::create();
    Node* cameraNode = _scene->addNode("Camera");
    cameraNode->setCamera(camera);
    _scene->setActiveCamera(camera);
    SAFE_RELEASE(camera);
    _formNodeParent = _scene->addNode("FormParent");
    _formNode = Node::create("Form");
    _formNodeParent->addChild(_formNode);
    
    formChanged();

    _gamepad = getGamepad(0);
    GP_ASSERT(_gamepad);
    _gamepad->getForm()->setConsumeInputEvents(false);
}

void FormsTest::formChanged()
{
    if (_activeForm)
        _activeForm->setEnabled(false);
    _activeForm = _forms[_formIndex];
    _activeForm->setEnabled(true);

    // Add the form to a node to allow it to be placed in 3D space.
    const Rectangle& bounds = _activeForm->getBounds();
    float scale;
    if (bounds.width >= bounds.height)
    {
        scale = 1.0f / bounds.width;
    }
    else
    {
        scale = 1.0f / bounds.height;
    }

    _formNode->setScale(scale, scale, 1.0f);
    _formNodeParent->setTranslation(0, 0, -1.5f);
    _formNode->setTranslation(-0.5f, -0.5f, 0);
    _formNode->setForm(_activeForm);
}

void FormsTest::createTestForm(Theme::Style* style)
{
    Form* form = Form::create("testForm", style);
    form->setSize(600, 600);

    Label* label = Label::create("testLabel", style);
    label->setPosition(0, 10);
    label->setSize(200, 200);
    label->setText("This is a label.");
    form->addControl(label);
    label->release();

    Button* button = Button::create("opacityButton", style);
    button->setPosition(0, 210);
    button->setSize(200, 200);
    button->setText("This is a button.  Click to change its opacity.");
    button->addListener(this, Control::Listener::CLICK);
    form->addControl(button);
    button->release();

    form->setEnabled(false);
    _forms.push_back(form);
}

void FormsTest::update(float elapsedTime)
{
    // Check if we have any physical gamepad connections.
    getGamepadsConnected();

    _gamepad->update(elapsedTime);

    float speedFactor = 0.001f * elapsedTime;
    bool aDown = (_keyFlags & KEY_A_MASK) || (_gamepad->getButtonState(BUTTON_A) == Gamepad::BUTTON_PRESSED);
    bool bDown = (_keyFlags & KEY_B_MASK) || (_gamepad->getButtonState(BUTTON_B) == Gamepad::BUTTON_PRESSED);
    Vector2 joyCommand;
    if (_gamepad->isJoystickActive(0))
    {
        _gamepad->getJoystickAxisValues(0, &joyCommand);
    }

    if (bDown)
    {
        _formNodeParent->setRotation(0, 0, 0, 1);
    }
    else if (aDown)
    {
        // Yaw in world frame, pitch in body frame
        Matrix m;
        _formNodeParent->getWorldMatrix().transpose(&m);
        Vector3 yaw;
        m.getUpVector(&yaw);
        _formNodeParent->rotate(yaw, speedFactor * joyCommand.x);
        _formNodeParent->rotateX(-speedFactor * joyCommand.y);
    }
    else
    {
        _formNodeParent->translate(0.5f * speedFactor * joyCommand.x, 0.5f * speedFactor * joyCommand.y, 0);
    }

    if (_formSelect)
    {
        _formSelect->update(elapsedTime);
    }

    if (_activeForm)
    {
        _activeForm->update(elapsedTime);
    }
}

void FormsTest::render(float elapsedTime)
{
    // Clear the screen.
    clear(CLEAR_COLOR_DEPTH, Vector4(0, 0, 0, 1), 1.0f, 0);

    // Draw the forms.
    if (_formSelect)
    {
        _formSelect->draw();
    }

    if (_activeForm)
    {
        _activeForm->draw();
    }

    _gamepad->draw();
}

void FormsTest::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    if (_formNode)
    {
        switch (evt)
        {
        case Touch::TOUCH_PRESS:
            {
                _touched = true;
                _touchX = x;
            }
            break;
        case Touch::TOUCH_RELEASE:
            {
                _touched = false;
                _touchX = 0;
            }
            break;
        case Touch::TOUCH_MOVE:
            {
                int deltaX = x - _touchX;
                _touchX = x;
                // Yaw in world frame
                Matrix m;
                _formNodeParent->getWorldMatrix().transpose(&m);
                Vector3 yaw;
                m.getUpVector(&yaw);
                _formNodeParent->rotate(yaw, MATH_DEG_TO_RAD(deltaX * 0.5f));
            }
            break;
        default:
            break;
        };
    }
}

void FormsTest::keyEvent(Keyboard::KeyEvent keyEvent, int key)
{
    if (_formNode)
    {
        switch(keyEvent)
        {
        case Keyboard::KEY_PRESS:
            switch (key)
            {
            case Keyboard::KEY_LEFT_ARROW:
                _formNodeParent->translateX(-0.1f);
                break;
            case Keyboard::KEY_RIGHT_ARROW:
                _formNodeParent->translateX(0.1f);
                break;
            case Keyboard::KEY_UP_ARROW:
                _formNodeParent->translateY(0.1f);
                break;
            case Keyboard::KEY_DOWN_ARROW:
                _formNodeParent->translateY(-0.1f);
                break;
            case Keyboard::KEY_PLUS:
                _formNodeParent->translateZ(0.1f);
                break;
            case Keyboard::KEY_MINUS:
                _formNodeParent->translateZ(-0.1f);
                break;
            case Keyboard::KEY_A:
            case Keyboard::KEY_CAPITAL_A:
                _keyFlags |= KEY_A_MASK;
                break;
            case Keyboard::KEY_B:
            case Keyboard::KEY_CAPITAL_B:
                _keyFlags |= KEY_B_MASK;
                break;
            }
            break;
        case Keyboard::KEY_RELEASE:
            switch (key)
            {
            case Keyboard::KEY_A:
            case Keyboard::KEY_CAPITAL_A:
                _keyFlags &= ~KEY_A_MASK;
                break;
            case Keyboard::KEY_B:
            case Keyboard::KEY_CAPITAL_B:
                _keyFlags &= ~KEY_B_MASK;
                break;
            }
            break;
        }
    }
}

void FormsTest::controlEvent(Control* control, EventType evt)
{
    if (strcmp("form0", control->getId()) == 0)
    {
        _formIndex = 0;
        formChanged();
    }
    else if (strcmp("form1", control->getId()) == 0)
    {
        _formIndex = 1;
        formChanged();
    }
    else if (strcmp("form2", control->getId()) == 0)
    {
        _formIndex = 2;
        formChanged();
    }
    else if (strcmp("form3", control->getId()) == 0)
    {
        _formIndex = 3;
        formChanged();
    }
    else if (strcmp("form4", control->getId()) == 0)
    {
        _formIndex = 4;
        formChanged();
    }
    else if (strcmp("form5", control->getId()) == 0)
    {
        _formIndex = 5;
        formChanged();
    }
    else if (strcmp("opacityButton", control->getId()) == 0)
    {
        //control->setOpacity(0.5f);

        float from[] = { 1.0f };
        float to[] = { 0.1f };
        control->createAnimationFromTo("opacityButton", Form::ANIMATE_OPACITY, from, to, Curve::LINEAR, 1000)->getClip()->play();
    }
}
