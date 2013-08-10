/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2009      Valentin Milea
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
#include "CCNode.h"
#include "CCDirector.h"
#include "platform/CCCommon.h"
#include <algorithm>

#if CC_NODE_RENDER_SUBPIXEL
#define RENDER_IN_SUBPIXEL
#else
#define RENDER_IN_SUBPIXEL(__ARGS__) (ceil(__ARGS__))
#endif

NS_CC_BEGIN

CCNode::CCNode(void)
: m_fScale(1.0f)
, m_obPosition(CCPointZero)
, m_obAnchorPointInPoints(CCPointZero)
, m_obAnchorPoint(CCPointZero)
, m_obContentSize(CCSizeZero)
// children (lazy allocs)
// lazy alloc
, m_nZOrder(0)
, m_pParent(NULL)
// "whole screen" objects. like Scenes and Layers, should set m_bIgnoreAnchorPointForPosition to true
, m_nTag(kCCNodeTagInvalid)
// userData is always inited as nil
, m_pUserData(NULL)
, m_bVisible(true)
, m_bIgnoreAnchorPointForPosition(false)
, m_bReorderChildDirty(false)
, m_Opacity(1.0f)
, m_Color(1.0f,1.0f,1.0f)
{
}

CCNode::~CCNode(void)
{
    CCLOGINFO( "cocos2d: deallocing" );

	for( unsigned int i = 0, s = m_Children.size();  i<s; ++i)
		delete m_Children[i];
}

bool CCNode::init()
{
    return true;
}

/// zOrder getter
int CCNode::getZOrder()
{
    return m_nZOrder;
}

/// zOrder setter : private method
/// used internally to alter the zOrder variable. DON'T call this method manually 
void CCNode::_setZOrder(int z)
{
    m_nZOrder = z;
}

void CCNode::setZOrder(int z)
{
    _setZOrder(z);
    if (m_pParent)
    {
        m_pParent->reorderChild(this, z);
    }
}

/// scale getter
float CCNode::getScale(void)
{
    return m_fScale;
}

/// scale setter
void CCNode::setScale(float scale)
{
    m_fScale= scale;
}


/// position getter
const CCPoint& CCNode::getPosition()
{
    return m_obPosition;
}

/// position setter
void CCNode::setPosition(const CCPoint& newPosition)
{
    m_obPosition = newPosition;
}

void CCNode::getPosition(float* x, float* y)
{
    *x = m_obPosition.x;
    *y = m_obPosition.y;
}

void CCNode::setPosition(float x, float y)
{
    setPosition(ccp(x, y));
}

float CCNode::getPositionX(void)
{
    return m_obPosition.x;
}

float CCNode::getPositionY(void)
{
    return  m_obPosition.y;
}

void CCNode::setPositionX(float x)
{
    setPosition(ccp(x, m_obPosition.y));
}

void CCNode::setPositionY(float y)
{
    setPosition(ccp(m_obPosition.x, y));
}

unsigned int CCNode::getChildrenCount(void) const
{
    return m_Children.size();
}


/// isVisible getter
bool CCNode::isVisible()
{
    return m_bVisible;
}

/// isVisible setter
void CCNode::setVisible(bool var)
{
    m_bVisible = var;
}

const CCPoint& CCNode::getAnchorPointInPoints()
{
    return m_obAnchorPointInPoints;
}

/// anchorPoint getter
const CCPoint& CCNode::getAnchorPoint()
{
    return m_obAnchorPoint;
}

void CCNode::setAnchorPoint(const CCPoint& point)
{
    if( ! point.equals(m_obAnchorPoint))
    {
        m_obAnchorPoint = point;
        m_obAnchorPointInPoints = ccp(m_obContentSize.width * m_obAnchorPoint.x, m_obContentSize.height * m_obAnchorPoint.y );
    }
}

/// contentSize getter
const CCSize& CCNode::getContentSize() const
{
    return m_obContentSize;
}

void CCNode::setContentSize(const CCSize & size)
{
    if ( ! size.equals(m_obContentSize))
    {
        m_obContentSize = size;

        m_obAnchorPointInPoints = ccp(m_obContentSize.width * m_obAnchorPoint.x, m_obContentSize.height * m_obAnchorPoint.y );
    }
}

/// parent getter
CCNode * CCNode::getParent()
{
    return m_pParent;
}
/// parent setter
void CCNode::setParent(CCNode * var)
{
    m_pParent = var;
}

/// isRelativeAnchorPoint getter
bool CCNode::isIgnoreAnchorPointForPosition()
{
    return m_bIgnoreAnchorPointForPosition;
}
/// isRelativeAnchorPoint setter
void CCNode::ignoreAnchorPointForPosition(bool newValue)
{
    if (newValue != m_bIgnoreAnchorPointForPosition) 
    {
		m_bIgnoreAnchorPointForPosition = newValue;
	}
}

/// tag getter
int CCNode::getTag() const
{
    return m_nTag;
}

/// tag setter
void CCNode::setTag(int var)
{
    m_nTag = var;
}

/// userData getter
void * CCNode::getUserData()
{
    return m_pUserData;
}

/// userData setter
void CCNode::setUserData(void *var)
{
    m_pUserData = var;
}

CCNode * CCNode::create(void)
{
	CCNode * pRet = new CCNode();
    if (pRet && pRet->init())
    {
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
	return pRet;
}

void CCNode::cleanup()
{
}

CCNode* CCNode::getChildByTag(int aTag)
{
    CCAssert( aTag != kCCNodeTagInvalid, "Invalid tag");

	for( unsigned int i=0, s=m_Children.size(); i<s; ++i )
    {
		CCNode* pNode = (CCNode*) m_Children[i];
		if(pNode && pNode->m_nTag == aTag)
			return pNode;
    }
    return NULL;
}

/* "add" logic MUST only be on this method
* If a class want's to extend the 'addChild' behavior it only needs
* to override this method
*/
void CCNode::addChild(CCNode *child, int zOrder, int tag)
{    
    CCAssert( child != NULL, "Argument must be non-nil");
    CCAssert( child->m_pParent == NULL, "child already added. It can't be added again");

    this->insertChild(child, zOrder);

    child->m_nTag = tag;

    child->setParent(this);
}

void CCNode::addChild(CCNode *child, int zOrder)
{
    CCAssert( child != NULL, "Argument must be non-nil");
    this->addChild(child, zOrder, child->m_nTag);
}

void CCNode::addChild(CCNode *child)
{
    CCAssert( child != NULL, "Argument must be non-nil");
    this->addChild(child, child->m_nZOrder, child->m_nTag);
}

void CCNode::removeFromParent()
{
    this->removeFromParentAndCleanup(true);
}

void CCNode::removeFromParentAndCleanup(bool cleanup)
{
    if (m_pParent != NULL)
    {
        m_pParent->removeChild(this,cleanup);
    } 
}

void CCNode::removeChild(CCNode* child)
{
    this->removeChild(child, true);
}

/* "remove" logic MUST only be on this method
* If a class want's to extend the 'removeChild' behavior it only needs
* to override this method
*/
void CCNode::removeChild(CCNode* child, bool cleanup)
{
	for( unsigned int i=0, s=m_Children.size(); i<s; ++i )
    {
		CCNode* pNode = (CCNode*) m_Children[i];
		if ( pNode == child )
		{
	        this->detachChild(child,cleanup);
			return;
		}
	}
}

void CCNode::removeChildByTag(int tag)
{
    this->removeChildByTag(tag, true);
}

void CCNode::removeChildByTag(int tag, bool cleanup)
{
    CCAssert( tag != kCCNodeTagInvalid, "Invalid tag");

    CCNode *child = this->getChildByTag(tag);

    if (child == NULL)
    {
        CCLOG("cocos2d: removeChildByTag(tag = %d): child not found!", tag);
    }
    else
    {
        this->removeChild(child, cleanup);
    }
}

void CCNode::removeAllChildren()
{
    this->removeAllChildrenWithCleanup(true);
}

void CCNode::removeAllChildrenWithCleanup(bool cleanup)
{
    // not using detachChild improves speed here
	for( unsigned int i=0, s=m_Children.size(); i<s; ++i )
    {
		CCNode* pNode = (CCNode*) m_Children[i];
        if (pNode)
        {
            if (cleanup)
            {
                pNode->cleanup();
            }
			delete pNode;
        }
        
        m_Children.clear();
    }
}

void CCNode::detachChild(CCNode *child, bool doCleanup)
{
    // If you don't do cleanup, the child's actions will not get removed and the
    // its scheduledSelectors_ dict will not get released!
    if (doCleanup)
    {
        child->cleanup();
    }

    // set parent nil at the end
    child->setParent(NULL);

    // not using detachChild improves speed here
	for( unsigned int i=0, s=m_Children.size(); i<s; ++i )
    {
		CCNode* pNode = (CCNode*) m_Children[i];
        if (pNode == child)
        {
			delete pNode;
			m_Children.erase( m_Children.begin() + i );
			break;
		}
	}
}


// helper used by reorderChild & add
void CCNode::insertChild(CCNode* child, int z)
{
    m_bReorderChildDirty = true;
	m_Children.push_back( child );
    child->_setZOrder(z);
}

void CCNode::reorderChild(CCNode *child, int zOrder)
{
    CCAssert( child != NULL, "Child must be non-nil");
    m_bReorderChildDirty = true;
    child->_setZOrder(zOrder);
}

void CCNode::sortAllChildren()
{
    if (m_bReorderChildDirty)
    {
		std::stable_sort( m_Children.begin(), m_Children.end() );
        m_bReorderChildDirty = false;
    }
}


 void CCNode::draw()
 {
     //CCAssert(0);
     // override me
     // Only use- this function to draw your stuff.
     // DON'T draw your stuff outside this method
 }

void CCNode::visit()
{
    // quick return if not visible. children won't be drawn.
    if (!m_bVisible)
    {
        return;
    }
//    kmGLPushMatrix();

    CCNode* pNode = NULL;
    unsigned int i = 0;

    if(m_Children.size() > 0)
    {
        sortAllChildren();
        // draw children zOrder < 0
		for( ; i < m_Children.size(); i++ )
        {
            pNode = (CCNode*) m_Children[i];

            if ( pNode && pNode->m_nZOrder < 0 ) 
            {
                pNode->visit();
            }
            else
            {
                break;
            }
        }
        // self draw
        this->draw();

        for( ; i < m_Children.size(); i++ )
        {
            pNode = (CCNode*) m_Children[i];
            if (pNode)
            {
                pNode->visit();
            }
        }        
    }
    else
    {
        this->draw();
    }

    //kmGLPopMatrix();
}

bool CCNode::callOnEvent( const CCEvent& e )
{
	if ( onEvent(e) )
		return true;

	for( unsigned int i=0, s=m_Children.size(); i<s; ++i )
		if ( m_Children[i]->callOnEvent( e ) )
			return true;

	return false;
}


// override me
void CCNode::update(float fDelta)
{
}

void CCNode::callUpdate(float fDelta)
{
	update( fDelta );
	for( unsigned int i=0, s=m_Children.size(); i<s; ++i )
		m_Children[i]->callUpdate(fDelta);
}


CCNodeTransform CCNode::nodeToParentTransform(void)
{
    // Translate values
    float x = m_obPosition.x;
    float y = m_obPosition.y;

    if (m_bIgnoreAnchorPointForPosition) 
    {
        x += m_obAnchorPointInPoints.x;
        y += m_obAnchorPointInPoints.y;
    }
      float cx = 1, sx = 0, cy = 1, sy = 0;
 
    // optimization:
    // inline anchor point calculation if skew is not needed
    // Adjusted transform calculation for rotational skew
    if (!m_obAnchorPointInPoints.equals(CCPointZero))
    {
        x += -m_obAnchorPointInPoints.x * m_fScale;
        y += -m_obAnchorPointInPoints.y * m_fScale;
    }

    return CCNodeTransform(x,y,m_fScale);
}

CCNodeTransform CCNode::parentToNodeTransform(void)
{
	return nodeToParentTransform().invert();
}

CCNodeTransform CCNode::nodeToWorldTransform()
{
    CCNodeTransform t = this->nodeToParentTransform();

    for (CCNode *p = m_pParent; p != NULL; p = p->getParent())
        t *= p->nodeToParentTransform();

    return t;
}

CCNodeTransform CCNode::worldToNodeTransform(void)
{
    return this->nodeToWorldTransform().invert();
}

CCPoint CCNode::convertToNodeSpace(const CCPoint& worldPoint)
{
	return worldToNodeTransform().apply(worldPoint);
}

CCPoint CCNode::convertToWorldSpace(const CCPoint& nodePoint)
{
	return nodeToWorldTransform().apply(nodePoint);
}


NS_CC_END
