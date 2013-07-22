#include "CCEGLViewProtocol.h"
#include "CCDirector.h"
#include <map>
#include <set>

NS_CC_BEGIN

static unsigned int s_indexBitsUsed = 0;
static std::map<int,int> s_TouchesIntergerDict;

static int getUnUsedIndex()
{
    int i;
    int temp = s_indexBitsUsed;

    for (i = 0; i < CC_MAX_TOUCHES; i++) {
        if (! (temp & 0x00000001)) {
            s_indexBitsUsed |= (1 <<  i);
            return i;
        }

        temp >>= 1;
    }

    // all bits are used
    return -1;
}

static void removeUsedIndexBit(int index)
{
    if (index < 0 || index >= CC_MAX_TOUCHES) 
    {
        return;
    }

    unsigned int temp = 1 << index;
    temp = ~temp;
    s_indexBitsUsed &= temp;
}

CCEGLViewProtocol::CCEGLViewProtocol()
: m_pDelegate(NULL)
, m_fScaleX(1.0f)
, m_fScaleY(1.0f)
{
}

CCEGLViewProtocol::~CCEGLViewProtocol()
{

}


const CCSize& CCEGLViewProtocol::getFrameSize() const
{
    return m_obScreenSize;
}

void CCEGLViewProtocol::setFrameSize(float width, float height)
{
    m_obScreenSize = CCSizeMake(width, height);
}

void CCEGLViewProtocol::setTouchDelegate(void * pDelegate)
{
    m_pDelegate = pDelegate;
}

void CCEGLViewProtocol::setViewName(const char* pszViewName)
{
    if (pszViewName != NULL && strlen(pszViewName) > 0)
    {
        strcpy_s(m_szViewName, pszViewName);
    }
}

const char* CCEGLViewProtocol::getViewName()
{
    return m_szViewName;
}

void CCEGLViewProtocol::handleTouchesBegin(int num, int ids[], float xs[], float ys[])
{
    for (int i = 0; i < num; ++i)
    {
        int id = ids[i];
        float x = xs[i];
        float y = ys[i];

		std::map<int,int>::iterator pIndex = s_TouchesIntergerDict.find(id);
        int nUnusedIndex = 0;

        // it is a new touch
        if (pIndex == s_TouchesIntergerDict.end())
        {
            nUnusedIndex = getUnUsedIndex();

            // The touches is more than MAX_TOUCHES ?
            if (nUnusedIndex == -1) {
                CCLOG("The touches is more than MAX_TOUCHES, nUnusedIndex = %d", nUnusedIndex);
                continue;
            }

			s_TouchesIntergerDict[id] = nUnusedIndex;

			CCEvent touchEvent;
			touchEvent.Type = CCEvent::TouchBegan;
			touchEvent.Touch.TouchId = nUnusedIndex;
			touchEvent.Touch.X = (x - m_obViewPortRect.origin.x) / m_fScaleX;// * scaleFactor;
			touchEvent.Touch.Y = (y - m_obViewPortRect.origin.y) / m_fScaleY;// * scaleFactor;
		
			CCDirector::sharedDirector()->pushEvent(touchEvent);
        }
    }
}

void CCEGLViewProtocol::handleTouchesMove(int num, int ids[], float xs[], float ys[])
{
    for (int i = 0; i < num; ++i)
    {
        int id = ids[i];
        float x = xs[i];
        float y = ys[i];

		std::map<int,int>::iterator pIndex = s_TouchesIntergerDict.find(id);

        if (pIndex == s_TouchesIntergerDict.end()) {
            CCLOG("if the index doesn't exist, it is an error");
            continue;
        }


        CCLOGINFO("Moving touches with id: %d, x=%f, y=%f", id, x, y);

		CCEvent touchEvent;
		touchEvent.Type = CCEvent::TouchMoved;
		touchEvent.Touch.TouchId = pIndex->second;
		touchEvent.Touch.X = (x - m_obViewPortRect.origin.x) / m_fScaleX;// * scaleFactor;
		touchEvent.Touch.Y = (y - m_obViewPortRect.origin.y) / m_fScaleY;// * scaleFactor;
		
		CCDirector::sharedDirector()->pushEvent(touchEvent);

            
    }
}

void CCEGLViewProtocol::handleTouchesEndOrCancel(bool isEnd, int num, int ids[], float xs[], float ys[])
{
    for (int i = 0; i < num; ++i)
    {
        int id = ids[i];
        float x = xs[i];
        float y = ys[i];

		std::map<int,int>::iterator pIndex = s_TouchesIntergerDict.find(id);

        if (pIndex == s_TouchesIntergerDict.end()) {
            CCLOG("if the index doesn't exist, it is an error");
            continue;
        }

        /* Add to the set to send to the director */
        {
            CCLOGINFO("Ending touches with id: %d, x=%f, y=%f", id, x, y);

			CCEvent touchEvent;
			touchEvent.Type = isEnd ? CCEvent::TouchEnded : CCEvent::TouchCancelled;
			touchEvent.Touch.TouchId = pIndex->second;
			touchEvent.Touch.X = (x - m_obViewPortRect.origin.x) / m_fScaleX;// * scaleFactor;
			touchEvent.Touch.Y = (y - m_obViewPortRect.origin.y) / m_fScaleY;// * scaleFactor;
		
			CCDirector::sharedDirector()->pushEvent(touchEvent);

			removeUsedIndexBit(pIndex->second);

			s_TouchesIntergerDict.erase(pIndex);
        } 
    }
}

void CCEGLViewProtocol::handleTouchesEnd(int num, int ids[], float xs[], float ys[])
{
	handleTouchesEndOrCancel( true, num, ids, xs, ys);
}

void CCEGLViewProtocol::handleTouchesCancel(int num, int ids[], float xs[], float ys[])
{
	handleTouchesEndOrCancel( false, num, ids, xs, ys);
}

void CCEGLViewProtocol::handleMouseWheelMoved(int tick)
{
	CCEvent mouseEvent;
	mouseEvent.Type = CCEvent::MouseWheelMoved;
	mouseEvent.MouseWheel.Delta = tick;
	
	CCDirector::sharedDirector()->pushEvent(mouseEvent); 
}

void CCEGLViewProtocol::handleMouseButtonPressed(int x, int y, int button)
{
	CCEvent mouseEvent;
	mouseEvent.Type = CCEvent::MouseButtonPressed;
	mouseEvent.MouseButton.X = x;
	mouseEvent.MouseButton.Y = y;
	mouseEvent.MouseButton.Button = (Mouse::Button)button;
	
	CCDirector::sharedDirector()->pushEvent(mouseEvent); 
}

void CCEGLViewProtocol::handleMouseButtonReleased(int x, int y, int button)
{
	CCEvent mouseEvent;
	mouseEvent.Type = CCEvent::MouseButtonReleased;
	mouseEvent.MouseButton.X = x;
	mouseEvent.MouseButton.Y = y;
	mouseEvent.MouseButton.Button = (Mouse::Button)button;
	
	CCDirector::sharedDirector()->pushEvent(mouseEvent); 
}

void CCEGLViewProtocol::handleMouseMoved(int x, int y)
{
	CCEvent mouseEvent;
	mouseEvent.Type = CCEvent::MouseMoved;
	mouseEvent.MouseMove.X = x;
	mouseEvent.MouseMove.Y = y;
	
	CCDirector::sharedDirector()->pushEvent(mouseEvent); 
}

void CCEGLViewProtocol::handleKeyEvent(Key::Code key, bool pressed)
{
	CCEvent keyEvent;
	keyEvent.Type = pressed ? CCEvent::KeyPressed : CCEvent::KeyReleased;
	keyEvent.Key.Alt = false;
	keyEvent.Key.Control = false;
	keyEvent.Key.Shift = false;
	keyEvent.Key.Code = key;
	
	CCDirector::sharedDirector()->pushEvent(keyEvent); 
}



const CCRect& CCEGLViewProtocol::getViewPortRect() const
{
    return m_obViewPortRect;
}

float CCEGLViewProtocol::getScaleX() const
{
    return m_fScaleX;
}

float CCEGLViewProtocol::getScaleY() const
{
    return m_fScaleY;
}

NS_CC_END
