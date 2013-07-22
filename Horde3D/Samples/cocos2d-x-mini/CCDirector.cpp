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

// standard includes
#include <string>

// cocos2d includes
#include "CCDirector.h"
#include "platform/platform.h"
#include "platform/CCFileUtils.h"
#include "support/user_default/CCUserDefault.h"
#include "nodes/CCNode.h"
#include "CCApplication.h"
#include "CCAccelerometer.h"
#include "CCEGLView.h"
#include "Horde3D.h"
#include <fstream>


/**
 Position of the FPS
 
 Default: 0,0 (bottom-left corner)
 */
#ifndef CC_DIRECTOR_STATS_POSITION
#define CC_DIRECTOR_STATS_POSITION CCDirector::sharedDirector()->getVisibleOrigin()
#endif

using namespace std;

unsigned int g_uNumberOfDraws = 0;

NS_CC_BEGIN
// XXX it should be a Director ivar. Move it there once support for multiple directors is added

// singleton stuff
static CCDisplayLinkDirector *s_SharedDirector = NULL;

#define kDefaultFPS        60  // 60 frames per second
extern const char* cocos2dVersion(void);

CCDirector* CCDirector::sharedDirector(void)
{
    if (!s_SharedDirector)
    {
        s_SharedDirector = new CCDisplayLinkDirector();
        s_SharedDirector->init();
    }

    return s_SharedDirector;
}

CCDirector::CCDirector(void)
{

}

bool CCDirector::init(void)
{
	setDefaultValues();

    m_pRootNode = NULL;

    // FPS
    m_fAccumDt = 0.0f;
    m_fFrameRate = 0.0f;
    m_uTotalFrames = m_uFrames = 0;
    m_pLastUpdate = new struct cc_timeval();

    // paused ?
    m_bPaused = false;
   
    // purge ?
    m_bPurgeDirecotorInNextLoop = false;

    m_obWinSizeInPoints = CCSizeZero;    

    m_pobOpenGLView = NULL;

    m_fContentScaleFactor = 1.0f;

    // Accelerometer
    m_pAccelerometer = new CCAccelerometer();

    return true;
}
    
CCDirector::~CCDirector(void)
{
    CCLOG("cocos2d: deallocing CCDirector %p", this);

    CC_SAFE_DELETE(m_pAccelerometer);

    // delete m_pLastUpdate
    CC_SAFE_DELETE(m_pLastUpdate);

    s_SharedDirector = NULL;
}

void CCDirector::setDefaultValues(void)
{
	m_dOldAnimationInterval = m_dAnimationInterval = 1.0 / 60.0f;
	m_bDisplayStats = true;
}

void CCDirector::setRootNode( CCNode* n ) 
{ 
	delete m_pRootNode;
	m_pRootNode = n; 
}


// Draw the Scene
void CCDirector::drawScene(void)
{
    // calculate "global" dt
    calculateDeltaTime();

	if (m_pRootNode)
	{	
		while( !m_Events.empty() )
		{
			CCEvent e = m_Events.front();
			m_Events.pop();
			m_pRootNode->onEvent( e );
		}

		// update
		m_pRootNode->callUpdate( m_fDeltaTime );
		// draw
		m_pRootNode->visit();
	}

    if (m_bDisplayStats)
    {
        showStats();
    }

    m_uTotalFrames++;

    // swap buffers
    if (m_pobOpenGLView)
    {
        m_pobOpenGLView->swapBuffers();
    }
    
    if (m_bDisplayStats)
    {
        calculateMPF();
    }
}

void CCDirector::calculateDeltaTime(void)
{
    struct cc_timeval now;

    if (CCTime::gettimeofdayCocos2d(&now, NULL) != 0)
    {
        CCLOG("error in gettimeofday");
        m_fDeltaTime = 0;
        return;
    }

    // new delta time. Re-fixed issue #1277
    if (m_bNextDeltaTimeZero)
    {
        m_fDeltaTime = 0;
        m_bNextDeltaTimeZero = false;
    }
    else
    {
        m_fDeltaTime = (now.tv_sec - m_pLastUpdate->tv_sec) + (now.tv_usec - m_pLastUpdate->tv_usec) / 1000000.0f;
        m_fDeltaTime = MAX(0, m_fDeltaTime);
    }

#ifdef DEBUG
    // If we are debugging our code, prevent big delta time
    if(m_fDeltaTime > 0.2f)
    {
        m_fDeltaTime = 1 / 60.0f;
    }
#endif

    *m_pLastUpdate = now;
}
float CCDirector::getDeltaTime()
{
	return m_fDeltaTime;
}
void CCDirector::setOpenGLView(CCEGLView *pobOpenGLView)
{
    CCAssert(pobOpenGLView, "opengl view should not be null");

    if (m_pobOpenGLView != pobOpenGLView)
    {
        // EAGLView is not a CCObject
        delete m_pobOpenGLView; // [openGLView_ release]
        m_pobOpenGLView = pobOpenGLView;

        // set size
        m_obWinSizeInPoints = m_pobOpenGLView->getFrameSize(); 

		m_pobOpenGLView->setTouchDelegate(this);
	}
}

void CCDirector::setNextDeltaTimeZero(bool bNextDeltaTimeZero)
{
    m_bNextDeltaTimeZero = bNextDeltaTimeZero;
}


void CCDirector::purgeCachedData(void)
{
    CCFileUtils::sharedFileUtils()->purgeCachedEntries();
}


CCSize CCDirector::getWinSize(void)
{
    return m_obWinSizeInPoints;
}

CCSize CCDirector::getWinSizeInPixels()
{
    return CCSizeMake(m_obWinSizeInPoints.width * m_fContentScaleFactor, m_obWinSizeInPoints.height * m_fContentScaleFactor);
}

// scene management


void CCDirector::end()
{
    m_bPurgeDirecotorInNextLoop = true;
}

void CCDirector::purgeDirector()
{
    stopAnimation();

    // purge all managed caches
    CCFileUtils::purgeFileUtils();

    // cocos2d-x specific data structures
    CCUserDefault::purgeSharedUserDefault();

    // OpenGL view
    m_pobOpenGLView->end();
    m_pobOpenGLView = NULL;
}


void CCDirector::pause(void)
{
    if (m_bPaused)
    {
        return;
    }

    m_dOldAnimationInterval = m_dAnimationInterval;

    // when paused, don't consume CPU
    setAnimationInterval(1 / 4.0);
    m_bPaused = true;
}

void CCDirector::resume(void)
{
    if (! m_bPaused)
    {
        return;
    }

    setAnimationInterval(m_dOldAnimationInterval);

    if (CCTime::gettimeofdayCocos2d(m_pLastUpdate, NULL) != 0)
    {
        CCLOG("cocos2d: Director: Error in gettimeofday");
    }

    m_bPaused = false;
    m_fDeltaTime = 0;
}

static string cleanPath( string path )
{
	// Remove spaces at the beginning
	int cnt = 0;
	for( int i = 0; i < (int)path.length(); ++i )
	{
		if( path[i] != ' ' ) break;
		else ++cnt;
	}
	if( cnt > 0 ) path.erase( 0, cnt );

	// Remove slashes, backslashes and spaces at the end
	cnt = 0;
	for( int i = (int)path.length() - 1; i >= 0; --i )
	{
		if( path[i] != '/' && path[i] != '\\' && path[i] != ' ' ) break;
		else ++cnt;
	}

	if( cnt > 0 ) path.erase( path.length() - cnt, cnt );

	return path;
}

static void splitPath(const std::string& full, std::string& path, std::string& nameWithExtension)
{
	// Remove slashes, backslashes and spaces at the end
	for( int i = (int)full.length() - 1; i>0; --i)
	{
		if( full[i] == '/' || full[i] == '\\')
		{
			path = full.substr(0, i+1);
			nameWithExtension = full.substr(i+1, full.size()-i-1);
			return;
		}
	}

	path = "";
	nameWithExtension = full;
} 

void CCDirector::loadResourcesFromDisk( const char *contentDir, const char* platformSubDir )
{
	string dir;
	vector< string > dirs;

	// Split path string
	char *c = (char *)contentDir;
	do
	{
		if( *c != '|' && *c != '\0' )
			dir += *c;
		else
		{
			dir = cleanPath( dir );
			if( dir != "" ) dir += '/';
			dirs.push_back( dir );
			dir = "";
		}
	} while( *c++ != '\0' );
	
	// Get the first resource that needs to be loaded
	int res = h3dQueryUnloadedResource( 0 );
	
	char *dataBuf = 0;
	int bufSize = 0;

	while( res != 0 )
	{
		ifstream inf;

		// Loop over search paths and try to open files
		for( unsigned int i = 0; i < dirs.size(); ++i )
		{	
			//try to load in platform specific directory
			if (platformSubDir!=NULL && platformSubDir[0]!=0)
			{
				string resPath,resName;
				splitPath( h3dGetResName( res ), resPath, resName);

				string fileName = dirs[i] + resPath + platformSubDir + "/" + resName;
				inf.clear();
				inf.open( fileName.c_str(), ios::binary );
				if( inf.good() ) break;
			}

			//without suffix
			string fileName = dirs[i] + h3dGetResName( res );
			inf.clear();
			inf.open( fileName.c_str(), ios::binary );
			if( inf.good() ) break;
		}

		// Open resource file
		if( inf.good() ) // Resource file found
		{
			// Find size of resource file
			inf.seekg( 0, ios::end );
			int fileSize = inf.tellg();
			if( bufSize < fileSize  )
			{
				delete[] dataBuf;				
				dataBuf = new char[fileSize];
				if( !dataBuf )
				{
					bufSize = 0;
					continue;
				}
				bufSize = fileSize;
			}
			if( fileSize == 0 )	continue;
			// Copy resource file to memory
			inf.seekg( 0 );
			inf.read( dataBuf, fileSize );
			inf.close();
			// Send resource data to engine
			h3dLoadResource( res, dataBuf, fileSize );
		}
		else // Resource file not found
		{
			// Tell engine to use the dafault resource by using NULL as data pointer
			h3dLoadResource( res, 0x0, 0 );
		}
		// Get next unloaded resource
		res = h3dQueryUnloadedResource( 0 );
	}
	delete[] dataBuf;
}


// display the FPS using a LabelAtlas
// updates the FPS every frame
void CCDirector::showStats(void)
{
    m_uFrames++;
    m_fAccumDt += m_fDeltaTime;
    
    if (m_bDisplayStats)
    {
/*        if (m_pFPSLabel && m_pSPFLabel && m_pDrawsLabel)
        {
            if (m_fAccumDt > CC_DIRECTOR_STATS_INTERVAL)
            {
                sprintf(m_pszFPS, "%.3f", m_fSecondsPerFrame);
                m_pSPFLabel->setString(m_pszFPS);
                
                m_fFrameRate = m_uFrames / m_fAccumDt;
                m_uFrames = 0;
                m_fAccumDt = 0;
                
                sprintf(m_pszFPS, "%.1f", m_fFrameRate);
                m_pFPSLabel->setString(m_pszFPS);
                
                sprintf(m_pszFPS, "%4lu", (unsigned long)g_uNumberOfDraws);
                m_pDrawsLabel->setString(m_pszFPS);
            }
        }
*/
    }    
    
    g_uNumberOfDraws = 0;
}

void CCDirector::calculateMPF()
{
    struct cc_timeval now;
    CCTime::gettimeofdayCocos2d(&now, NULL);
    
    m_fSecondsPerFrame = (now.tv_sec - m_pLastUpdate->tv_sec) + (now.tv_usec - m_pLastUpdate->tv_usec) / 1000000.0f;
}


float CCDirector::getContentScaleFactor(void)
{
    return m_fContentScaleFactor;
}

void CCDirector::setContentScaleFactor(float scaleFactor)
{
    if (scaleFactor != m_fContentScaleFactor)
    {
        m_fContentScaleFactor = scaleFactor;
    }
}

CCDirectorDelegate* CCDirector::getDelegate() const
{
    return m_pProjectionDelegate;
}

void CCDirector::setDelegate(CCDirectorDelegate* pDelegate)
{
    m_pProjectionDelegate = pDelegate;
}



void CCDirector::setAccelerometer(CCAccelerometer* pAccelerometer)
{
    if (m_pAccelerometer != pAccelerometer)
    {
        CC_SAFE_DELETE(m_pAccelerometer);
        m_pAccelerometer = pAccelerometer;
    }
}

CCAccelerometer* CCDirector::getAccelerometer()
{
    return m_pAccelerometer;
}

/***************************************************
* implementation of DisplayLinkDirector
**************************************************/

// should we implement 4 types of director ??
// I think DisplayLinkDirector is enough
// so we now only support DisplayLinkDirector
void CCDisplayLinkDirector::startAnimation(void)
{
    if (CCTime::gettimeofdayCocos2d(m_pLastUpdate, NULL) != 0)
    {
        CCLOG("cocos2d: DisplayLinkDirector: Error on gettimeofday");
    }

    m_bInvalid = false;
#ifndef EMSCRIPTEN
    CCApplication::sharedApplication()->setAnimationInterval(m_dAnimationInterval);
#endif // EMSCRIPTEN
}

void CCDisplayLinkDirector::mainLoop(void)
{
    if (m_bPurgeDirecotorInNextLoop)
    {
        m_bPurgeDirecotorInNextLoop = false;
        purgeDirector();
    }
    else if (! m_bInvalid)
     {
         drawScene();
     }
}

void CCDisplayLinkDirector::stopAnimation(void)
{
    m_bInvalid = true;
}

void CCDisplayLinkDirector::setAnimationInterval(double dValue)
{
    m_dAnimationInterval = dValue;
    if (! m_bInvalid)
    {
        stopAnimation();
        startAnimation();
    }    
}

NS_CC_END

