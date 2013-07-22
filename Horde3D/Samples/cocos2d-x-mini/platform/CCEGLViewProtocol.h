#ifndef __CCEGLVIEWPROTOCOL_H__
#define __CCEGLVIEWPROTOCOL_H__

#include "cocoa/ccGeometry.h"
#include "CCEvent.h"

NS_CC_BEGIN

#define CC_MAX_TOUCHES  5

class CCSet;

/**
 * @addtogroup platform
 * @{
 */

class CC_DLL CCEGLViewProtocol
{
public:
    CCEGLViewProtocol();
    virtual ~CCEGLViewProtocol();

    /** Force destroying EGL view, subclass must implement this method. */
    virtual void    end() = 0;

    /** Get whether opengl render system is ready, subclass must implement this method. */
    virtual bool    isOpenGLReady() = 0;

    /** Exchanges the front and back buffers, subclass must implement this method. */
    virtual void    swapBuffers() = 0;

    /** Open or close IME keyboard , subclass must implement this method. */
    virtual void    setIMEKeyboardState(bool bOpen) = 0;

    /**
     * Get the frame size of EGL view.
     * In general, it returns the screen size since the EGL view is a fullscreen view.
     */
    virtual const CCSize& getFrameSize() const;

    /**
     * Set the frame size of EGL view.
     */
    virtual void setFrameSize(float width, float height);

    /** Set touch delegate */
    virtual void setTouchDelegate(void* pDelegate);

    virtual void setViewName(const char* pszViewName);

    const char* getViewName();

		//touches dispatcher
    void handleTouchesBegin(int num, int ids[], float xs[], float ys[]);
    void handleTouchesMove(int num, int ids[], float xs[], float ys[]);
    void handleTouchesEnd(int num, int ids[], float xs[], float ys[]);
    void handleTouchesCancel(int num, int ids[], float xs[], float ys[]);
	void handleTouchesEndOrCancel(bool isEnd, int num, int ids[], float xs[], float ys[]);
	
		//mouse dispatcher
	void handleMouseWheelMoved(int tick); 
	void handleMouseButtonPressed(int x, int y, int button);
	void handleMouseButtonReleased(int x, int y, int button);
	void handleMouseMoved(int x, int y); 
		//keyboard dispatcher
	void handleKeyEvent(Key::Code key, bool pressed);

    /**
     * Get the opengl view port rectangle.
     */
    const CCRect& getViewPortRect() const;

    /**
     * Get scale factor of the horizontal direction.
     */
    float getScaleX() const;

    /**
     * Get scale factor of the vertical direction.
     */
    float getScaleY() const;
private:
    void getSetOfTouchesEndOrCancel(CCSet& set, int num, int ids[], float xs[], float ys[]);

protected:
    void* m_pDelegate;

    // real screen size
    CCSize m_obScreenSize;
    // the view port size
    CCRect m_obViewPortRect;
    // the view name
    char   m_szViewName[50];

    float  m_fScaleX;
    float  m_fScaleY;
};

// end of platform group
/// @}

NS_CC_END

#endif /* __CCEGLVIEWPROTOCOL_H__ */
