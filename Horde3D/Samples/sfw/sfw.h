#ifndef __sf_h_
#define __sf_h_

#ifdef __cplusplus
extern "C" {
#endif

	//these must be implemented by client app
void sfwMain(unsigned int w, unsigned int h, void* device);
void sfwUpdate(float elapsed);

struct sfwEvent;

	// public functions
bool sfwGetEvent(sfwEvent* e);	//query event, returns false in no more event
void sfwLoadResourcesFromDisk( const char *contentDir, const char* platformSubDir );

	// "private" functions
void sfwPushEvent(sfwEvent* e);	//adds event, used by platform implementations


////////////////////////////////////////////////////////////
/// Definition of key codes for keyboard events
////////////////////////////////////////////////////////////
struct sfwKey
{
    enum Code
    {
		/* Keyboard key code definitions: 8-bit ISO-8859-1 (Latin 1) encoding is used
		 * for printable keys (such as A-Z, 0-9 etc), and values above 256
		 * represent special (non-printable) keys (e.g. F1, Page Up etc).
		 */
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
        BackSlash,
        Return,
        Tab,
        PageUp,
        PageDown,
        End,
        Home,
        Insert,
        Delete,
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
};


struct sfwEventType
{
    enum Type
    {
		None,
        Resized,
        LostFocus,
        GainedFocus,
		Quit,
        KeyPressed,
        KeyReleased,
        PointerDown,	//mouse or touch events
        PointerUp,
		PointerCancelled,
        PointerMoved,
		ContextReset,
        Count
    };
};

////////////////////////////////////////////////////////////
/// Event type with parameters
////////////////////////////////////////////////////////////
struct sfwEvent
{
    ////////////////////////////////////////////////////////////
    /// event parameters types
    ////////////////////////////////////////////////////////////
    struct SizeEvent
    {
        unsigned int width;
        unsigned int height;
    };
	
	struct PointerEvent
	{
		float x;
		float y;
	};

	sfwEventType::Type type; ///< Type of the event

		/// event parameters
    union
    {
        sfwKey::Code       key;
        SizeEvent          size;
		PointerEvent       pointer;
    };
}; 

#ifdef __cplusplus
}
#endif

#endif