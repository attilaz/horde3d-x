/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __CCDIRECTOR_H__
#define __CCDIRECTOR_H__

#include "platform/CCPlatformMacros.h"
#include "CCEvent.h"
#include "include/ccTypes.h"
#include "cocoa/CCGeometry.h"
#include "include/ccTypeInfo.h"
#include <queue>  

NS_CC_BEGIN

/** @typedef ccDirectorProjection
 Possible OpenGL projections used by director
 */

/* Forward declarations. */
class CCEGLView;
class CCDirectorDelegate;
class CCNode;
class CCAccelerometer;

	//keypad dispatcher
typedef enum {
    // the back key clicked msg
    kTypeBackClicked = 1,
    kTypeMenuClicked,
} ccKeypadMSGType;

	//ime dispatcher
typedef struct
{
    CCRect  begin;              // the soft keyboard rectangle when animation begins
    CCRect  end;                // the soft keyboard rectangle when animation ends
    float     duration;           // the soft keyboard animation duration
} CCIMEKeyboardNotificationInfo;
 
 

class CC_DLL CCDirector : public TypeInfo
{
public:
    CCDirector(void);
    virtual ~CCDirector(void);
    virtual bool init(void);
    virtual long getClassTypeInfo() {
		static const long id = cocos2d::getHashCodeByString(typeid(cocos2d::CCDirector).name());
		return id;
    }

    // attribute

    /** Get the FPS value */
    inline double getAnimationInterval(void) { return m_dAnimationInterval; }
    /** Set the FPS value. */
    virtual void setAnimationInterval(double dValue) = 0;

    /** Whether or not to display the FPS on the bottom-left corner */
    inline bool isDisplayStats(void) { return m_bDisplayStats; }
    /** Display the FPS on the bottom-left corner */
    inline void setDisplayStats(bool bDisplayStats) { m_bDisplayStats = bDisplayStats; }
    
    /** seconds per frame */
    inline float getSecondsPerFrame() { return m_fSecondsPerFrame; }

    /** Get the CCEGLView, where everything is rendered */
    inline CCEGLView* getOpenGLView(void) { return m_pobOpenGLView; }
    void setOpenGLView(CCEGLView *pobOpenGLView);

    inline bool isNextDeltaTimeZero(void) { return m_bNextDeltaTimeZero; }
    void setNextDeltaTimeZero(bool bNextDeltaTimeZero);

    /** Whether or not the Director is paused */
    inline bool isPaused(void) { return m_bPaused; }

    /** How many frames were called since the director started */
    inline unsigned int getTotalFrames(void) { return m_uTotalFrames; }
    
    /** CCDirector delegate. It shall implemente the CCDirectorDelegate protocol
     @since v0.99.5
     */
    CCDirectorDelegate* getDelegate() const;
    void setDelegate(CCDirectorDelegate* pDelegate);

	void loadResourcesFromDisk( const char *contentDir, const char* platformSubDir );

    // window size

    /** returns the size of the OpenGL view in points.
    */
    CCSize getWinSize(void);

    /** returns the size of the OpenGL view in pixels.
    */
    CCSize getWinSizeInPixels(void);
    
    /** Ends the execution, releases the running scene.
     It doesn't remove the OpenGL view from its parent. You have to do it manually.
     */
    void end(void);

    /** Pauses the running scene.
     The running scene will be _drawed_ but all scheduled timers will be paused
     While paused, the draw rate will be 4 FPS to reduce CPU consumption
     */
    void pause(void);

    /** Resumes the paused scene
     The scheduled timers will be activated again.
     The "delta time" will be 0 (as if the game wasn't paused)
     */
    void resume(void);

    /** Stops the animation. Nothing will be drawn. The main loop won't be triggered anymore.
     If you don't want to pause your animation call [pause] instead.
     */
    virtual void stopAnimation(void) = 0;

    /** The main loop is triggered again.
     Call this function only if [stopAnimation] was called earlier
     @warning Don't call this function to start the main loop. To run the main loop call runWithScene
     */
    virtual void startAnimation(void) = 0;

    /** Draw the scene.
    This method is called every frame. Don't call it manually.
    */
    void drawScene(void);

	void setRootNode( CCNode* n );

    // Memory Helper

    /** Removes cached all cocos2d cached data.
     It will purge the CCTextureCache, CCSpriteFrameCache, CCLabelBMFont cache
     @since v0.99.3
     */
    void purgeCachedData(void);

	/** sets the default values based on the CCConfiguration info */
    void setDefaultValues(void);

    virtual void mainLoop(void) = 0;

    /** The size in pixels of the surface. It could be different than the screen size.
    High-res devices might have a higher surface size than the screen size.
    Only available when compiled using SDK >= 4.0.
    @since v0.99.4
    */
    void setContentScaleFactor(float scaleFactor);
    float getContentScaleFactor(void);

    /** CCKeypadDispatcher associated with this director
     @since v2.0
     */
	CCDirector* getKeypadDispatcher() { return this; }

   /**
    @brief dispatch the key pad msg
    */
	bool dispatchKeypadMSG(ccKeypadMSGType nMsgType) { return false; } 

		// ime dispatcher
	void dispatchInsertText(const char * pText, int nLen) {}
    void dispatchDeleteBackward() {}

    /**
    @brief Get the content text from CCIMEDelegate, retrieved previously from IME.
    */
	const char * getContentText() { return ""; }

   // dispatch keyboard notification
    //////////////////////////////////////////////////////////////////////////
	void dispatchKeyboardWillShow(CCIMEKeyboardNotificationInfo& info) {}
    void dispatchKeyboardDidShow(CCIMEKeyboardNotificationInfo& info) {}
    void dispatchKeyboardWillHide(CCIMEKeyboardNotificationInfo& info) {}
    void dispatchKeyboardDidHide(CCIMEKeyboardNotificationInfo& info) {} 

	void pushEvent(const CCEvent& _event) { m_Events.push(_event); } 

public:
    /** CCAccelerometer associated with this director
     @since v2.0
     */
    CC_PROPERTY(CCAccelerometer*, m_pAccelerometer, Accelerometer);

    /* delta time since last tick to main loop */
	CC_PROPERTY_READONLY(float, m_fDeltaTime, DeltaTime);
	
public:
    /** returns a shared instance of the director */
    static CCDirector* sharedDirector(void);

protected:

    void purgeDirector();
    bool m_bPurgeDirecotorInNextLoop; // this flag will be set to true in end()
    
    void setNextScene(void);
    
    void showStats();
    void calculateMPF();
    
    /** calculates delta time since last time it was called */    
    void calculateDeltaTime();
protected:
    /* The CCEGLView, where everything is rendered */
    CCEGLView    *m_pobOpenGLView;

    double m_dAnimationInterval;
    double m_dOldAnimationInterval;

    /* landscape mode ? */
    bool m_bLandscape;
    
    bool m_bDisplayStats;
    float m_fAccumDt;
    float m_fFrameRate;
    
    /** Whether or not the Director is paused */
    bool m_bPaused;

    /* How many frames were called since the director started */
    unsigned int m_uTotalFrames;
    unsigned int m_uFrames;
    float m_fSecondsPerFrame;
     
	CCNode*		m_pRootNode;
   
    /* last time the main loop was updated */
    struct cc_timeval *m_pLastUpdate;

    /* whether or not the next delta time will be zero */
    bool m_bNextDeltaTimeZero;
    
    /* window size in points */
    CCSize    m_obWinSizeInPoints;
    
    /* content scale factor */
    float    m_fContentScaleFactor;

    /* Projection protocol delegate */
    CCDirectorDelegate *m_pProjectionDelegate;
    
	std::queue<CCEvent> m_Events;         ///< Queue of received events  

    // CCEGLViewProtocol will recreate stats labels to fit visible rect
    friend class CCEGLViewProtocol;
};

#define CCIMEDispatcher CCDirector
#define sharedDispatcher sharedDirector
 

/** 
 @brief DisplayLinkDirector is a Director that synchronizes timers with the refresh rate of the display.
 
 Features and Limitations:
  - Scheduled timers & drawing are synchronizes with the refresh rate of the display
  - Only supports animation intervals of 1/60 1/30 & 1/15
 
 @since v0.8.2
 */
class CCDisplayLinkDirector : public CCDirector
{
public:
    CCDisplayLinkDirector(void) 
        : m_bInvalid(false)
    {}

    virtual void mainLoop(void);
    virtual void setAnimationInterval(double dValue);
    virtual void startAnimation(void);
    virtual void stopAnimation();

protected:
    bool m_bInvalid;
};

// end of base_node group
/// @}

NS_CC_END

#endif // __CCDIRECTOR_H__
