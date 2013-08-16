#pragma once

namespace cocos2d
{

////////////////////////////////////////////////////////////
/// Definition of key codes for keyboard events
////////////////////////////////////////////////////////////
namespace Key
{
    enum Code
    {
        A = 'a',
        B = 'b',
        C = 'c',
        D = 'd',
        E = 'e',
        F = 'f',
        G = 'g',
        H = 'h',
        I = 'i',
        J = 'j',
        K = 'k',
        L = 'l',
        M = 'm',
        N = 'n',
        O = 'o',
        P = 'p',
        Q = 'q',
        R = 'r',
        S = 's',
        T = 't',
        U = 'u',
        V = 'v',
        W = 'w',
        X = 'x',
        Y = 'y',
        Z = 'z',
        Num0 = '0',
        Num1 = '1',
        Num2 = '2',
        Num3 = '3',
        Num4 = '4',
        Num5 = '5',
        Num6 = '6',
        Num7 = '7',
        Num8 = '8',
        Num9 = '9', 
        Escape = 256,
        LControl,
        LShift,
        LAlt,
        LSystem,      ///< OS specific key (left side) : windows (Win and Linux), apple (MacOS), ...
        RControl,
        RShift,
        RAlt,
        RSystem,      ///< OS specific key (right side) : windows (Win and Linux), apple (MacOS), ...
        Menu,
        LBracket,     ///< [
        RBracket,     ///< ]
        SemiColon,    ///< ;
        Comma,        ///< ,
        Period,       ///< .
        Quote,        ///< '
        Slash,        ///< /
        BackSlash,
        Tilde,        ///< ~
        Equal,        ///< =
        Dash,         ///< -
        Space,
        Return,
        Back,
        Tab,
        PageUp,
        PageDown,
        End,
        Home,
        Insert,
        Delete,
        Add,          ///< +
        Subtract,     ///< -
        Multiply,     ///< *
        Divide,       ///< /
        Left,         ///< Left arrow
        Right,        ///< Right arrow
        Up,           ///< Up arrow
        Down,         ///< Down arrow
        Numpad0,
        Numpad1,
        Numpad2,
        Numpad3,
        Numpad4,
        Numpad5,
        Numpad6,
        Numpad7,
        Numpad8,
        Numpad9,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        Pause,

        Count // Keep last -- total number of keyboard keys
    };
}


////////////////////////////////////////////////////////////
/// Definition of button codes for mouse events
////////////////////////////////////////////////////////////
namespace Mouse
{
    enum Button
    {
        Left,
        Right,
        Middle,
        XButton1,
        XButton2,

        ButtonCount // Keep last -- total number of mouse buttons
    };
}


////////////////////////////////////////////////////////////
/// Definition of joystick axis for joystick events
////////////////////////////////////////////////////////////
namespace Joy
{
    enum Axis
    {
        AxisX,
        AxisY,
        AxisZ,
        AxisR,
        AxisU,
        AxisV,
        AxisPOV,

        AxisCount // Keep last -- total number of joystick axis
    };

    enum
    {
        Count       = 4, ///< Total number of supported joysticks
        ButtonCount = 32 ///< Total number of supported joystick buttons
    };
}
	
	
namespace Accel
{
	enum Axis
	{
		AxisX,
        AxisY,
        AxisZ,
		
		Count // For internal use
	};
}


////////////////////////////////////////////////////////////
/// Event defines a system event and its parameters
////////////////////////////////////////////////////////////
class CCEvent
{
public :

    ////////////////////////////////////////////////////////////
    /// Keyboard event parameters
    ////////////////////////////////////////////////////////////
    struct KeyEvent
    {
        Key::Code Code;
        bool      Alt;
        bool      Control;
        bool      Shift;
    };

    ////////////////////////////////////////////////////////////
    /// Text event parameters
    ////////////////////////////////////////////////////////////
    struct TextEvent
    {
        unsigned int Unicode;	//uint32
    };

    ////////////////////////////////////////////////////////////
    /// Mouse move event parameters
    ////////////////////////////////////////////////////////////
    struct MouseMoveEvent
    {
        int X;
        int Y;
    };

    ////////////////////////////////////////////////////////////
    /// Mouse buttons events parameters
    ////////////////////////////////////////////////////////////
    struct MouseButtonEvent
    {
        Mouse::Button Button;
        int           X;
        int           Y;
    };

    ////////////////////////////////////////////////////////////
    /// Mouse wheel events parameters
    ////////////////////////////////////////////////////////////
    struct MouseWheelEvent
    {
        int Delta;
    };

    ////////////////////////////////////////////////////////////
    /// Joystick axis move event parameters
    ////////////////////////////////////////////////////////////
    struct JoyMoveEvent
    {
        unsigned int JoystickId;
        Joy::Axis    Axis;
        float        Position;
    };

    ////////////////////////////////////////////////////////////
    /// Joystick buttons events parameters
    ////////////////////////////////////////////////////////////
    struct JoyButtonEvent
    {
        unsigned int JoystickId;
        unsigned int Button;
    };

    ////////////////////////////////////////////////////////////
    /// Size events parameters
    ////////////////////////////////////////////////////////////
    struct SizeEvent
    {
        unsigned int Width;
        unsigned int Height;
    };
	
	
	////////////////////////////////////////////////////////////
    /// Accelerometer event parameters
    ////////////////////////////////////////////////////////////
	struct AccelerometerEvent
	{
		float X;
		float Y;
		float Z;
	};
	
	////////////////////////////////////////////////////////////
    /// Touch event parameters
    ////////////////////////////////////////////////////////////
	struct TouchEvent
	{
		int TouchId;
		
		float X;
		float Y;
	};

    ////////////////////////////////////////////////////////////
    /// Enumeration of the different types of events
    ////////////////////////////////////////////////////////////
    enum EventType
    {
		Quit,
        Closed,
        Resized,
        LostFocus,
        GainedFocus,
        TextEntered,
        KeyPressed,
        KeyReleased,
        MouseWheelMoved,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseEntered,
        MouseLeft,
        JoyButtonPressed,
        JoyButtonReleased,
        JoyMoved,
		Accelerometer,
		TouchBegan,
		TouchEnded,
		TouchCancelled,
		TouchMoved,
        Count // Keep last -- total number of event types
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    EventType Type; ///< Type of the event

    union
    {
        KeyEvent           Key;
        TextEvent          Text;
        MouseMoveEvent     MouseMove;
        MouseButtonEvent   MouseButton;
        MouseWheelEvent    MouseWheel;
        JoyMoveEvent       JoyMove;
        JoyButtonEvent     JoyButton;
        SizeEvent          Size;
		AccelerometerEvent Acceleration;
		TouchEvent         Touch;
    };
};

} // namespace cocos2d


