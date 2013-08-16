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

#ifndef __PLATFORM_CCNODE_H__
#define __PLATFORM_CCNODE_H__

#include "platform/CCPlatformMacros.h"
#include "cocoa/CCGeometry.h"
#include "CCEvent.h"
#include <vector>

NS_CC_BEGIN

class CCPoint;
class CCTouch;

class CCColor3F
{
public:
	CCColor3F(float r, float g, float b) : r(r), g(g), b(b)
	{}

	float r,g,b;
};

class CCNodeTransform
{
public:
	CCNodeTransform(float tx, float ty, float s) : tx(tx), ty(ty), s(s)
	{}

	CCNodeTransform invert( )
	{
		float invs = 1.0f / s;
		return CCNodeTransform( -tx * invs, -tx * invs,  invs);
	}

	CCNodeTransform& operator*=(const CCNodeTransform& tr)
	{
		tx = tx * tr.s + tr.tx;
		ty = ty * tr.s + tr.ty;
		s = tr.s * s;
		return *this;
	}

	CCPoint apply(const CCPoint& p)
	{
		return CCPoint( p.x * tx + s, p.y * ty + s);
	}

	float tx,ty;	// translate x,y
	float s;		// scale
};

/**
 * @addtogroup base_nodes
 * @{
 */

enum {
    kCCNodeTagInvalid = -1,
};



/** @brief CCNode is the main element. 

 The main features of a CCNode are:
 - They can contain other CCNode nodes (addChild, getChildByTag, removeChild, etc)

 Some CCNode nodes provide extra functionality for them or their children.

 Subclassing a CCNode usually means (one/all) of:
 - overriding init to initialize resources and schedule callbacks
 - create callbacks to handle the advancement of time
 - overriding draw to render the node

 Features of CCNode:
 - position
 - scale (uniform)
 - anchor point
 - size
 - visible
 - z-order
 - openGL z position
 - opacity
 - RGB colors

 Default values:
 - position: (x=0,y=0)
 - scale: (1)
 - contentSize: (x=0,y=0)
 - anchorPoint: (x=0,y=0)
 - opacity (1.0f)
 - RGB colors (1.0f,1.0f,1.0f)

 Limitations:
 - A CCNode is a "void" object. It doesn't have a texture

 Order in transformations with grid disabled
 -# The node will be translated (position)
 -# The node will be scaled (scale)
 -# The node will be moved according to the camera values (camera)
 */

class CC_DLL CCNode
{
public:
    /// @{
    /// @name Constructor, Distructor and Initializers
    
    /**
     * Default constructor
     */
    CCNode(void);
    
    /**
     * Default destructor
     */
    virtual ~CCNode(void);
    
    /**
     *  Initializes the instance of CCNode
     *  @return Whether the initialization was successful.
     */
    virtual bool init();
	/**
     * Allocates and initializes a node.
     * @return A initialized node which is marked as "autorelease".
     */
    static CCNode * create(void);
    
    /// @} end of initializers
    
    
    
    /// @{
    /// @name Setters & Getters for Graphic Peroperties
    
    /**
     * Sets the Z order which stands for the drawing order, and reorder this node in its parent's children array.
     *
     * The Z order of node is relative to its "brothers": children of the same parent.
     * It's nothing to do with OpenGL's z vertex. This one only affects the draw order of nodes in cocos2d.
     * The larger number it is, the later this node will be drawn in each message loop.
     * Please refer to setVertexZ(float) for the difference.
     *
     * @param nZOrder   Z order of this node.
     */
    virtual void setZOrder(int zOrder);
    /**
     * Sets the z order which stands for the drawing order
     *
     * This is an internal method. Don't call it outside the framework.
     * The difference between setZOrder(int) and _setOrder(int) is:
     * - _setZOrder(int) is a pure setter for m_nZOrder memeber variable
     * - setZOrder(int) firstly changes m_nZOrder, then recorder this node in its parent's chilren array.
     */
    virtual void _setZOrder(int z);
    /**
     * Gets the Z order of this node.
     *
     * @see setZOrder(int)
     *
     * @return The Z order.
     */
    virtual int getZOrder();
    
    /**
     * Changes both X and Y scale factor of the node.
     *
     * 1.0 is the default scale factor. It modifies the X and Y scale at the same time.
     *
     * @param scale     The scale factor for both X and Y axis.
     */
    virtual void setScale(float scale);
    /**
     * Gets the scale factor of the node,  when X and Y have the same scale factor.
     *
     * @see setScale(float)
     *
     * @return The scale factor of the node.
     */
    virtual float getScale();
    
    
    /**
     * Changes the position (x,y) of the node in OpenGL coordinates
     *
     * Usually we use ccp(x,y) to compose CCPoint object.
     * The original point (0,0) is at the left-bottom corner of screen.
     * For example, this codesnip sets the node in the center of screen.
     * @code
     * CCSize size = CCDirector::sharedDirector()->getWinSize();
     * node->setPosition( ccp(size.width/2, size.height/2) )
     * @endcode
     *
     * @param position  The position (x,y) of the node in OpenGL coordinates
     */
    virtual void setPosition(const CCPoint &position);
    /**
     * Gets the position (x,y) of the node in OpenGL coordinates
     * 
     * @see setPosition(const CCPoint&)
     *
     * @return The position (x,y) of the node in OpenGL coordinates
     */
    virtual const CCPoint& getPosition();
    /**
     * Sets position in a more efficient way.
     *
     * Passing two numbers (x,y) is much efficient than passing CCPoint object.
     * This method is binded to lua and javascript. 
     * Passing a number is 10 times faster than passing a object from lua to c++
     *
     * @code
     * // sample code in lua
     * local pos  = node::getPosition()  -- returns CCPoint object from C++
     * node:setPosition(x, y)            -- pass x, y coordinate to C++
     * @endcode
     *
     * @param x     X coordinate for position
     * @param y     Y coordinate for position
     */
    virtual void setPosition(float x, float y);
    /**
     * Gets position in a more efficient way, returns two number instead of a CCPoint object
     *
     * @see setPosition(float, float)
     */
    virtual void getPosition(float* x, float* y);
    /**
     * Gets/Sets x or y coordinate individually for position.
     * These methods are used in Lua and Javascript Bindings
     */
    virtual void  setPositionX(float x);
    virtual float getPositionX(void);
    virtual void  setPositionY(float y);
    virtual float getPositionY(void);
    
    /**
     * Sets the anchor point in percent.
     *
     * anchorPoint is the point around which all transformations and positioning manipulations take place.
     * It's like a pin in the node where it is "attached" to its parent.
     * The anchorPoint is normalized, like a percentage. (0,0) means the bottom-left corner and (1,1) means the top-right corner.
     * But you can use values higher than (1,1) and lower than (0,0) too.
     * The default anchorPoint is (0.5,0.5), so it starts in the center of the node.
     *
     * @param anchorPoint   The anchor point of node.
     */
    virtual void setAnchorPoint(const CCPoint& anchorPoint);
    /** 
     * Returns the anchor point in percent.
     *
     * @see setAnchorPoint(const CCPoint&)
     *
     * @return The anchor point of node.
     */
    virtual const CCPoint& getAnchorPoint();
    /**
     * Returns the anchorPoint in absolute pixels.
     * 
     * @warning You can only read it. If you wish to modify it, use anchorPoint instead.
     * @see getAnchorPoint()
     *
     * @return The anchor point in absolute pixels.
     */
    virtual const CCPoint& getAnchorPointInPoints();
    
    
    /**
     * Sets the untransformed size of the node.
     *
     * The contentSize remains the same no matter the node is scaled or rotated.
     * All nodes has a size. Layer and Scene has the same size of the screen.
     *
     * @param contentSize   The untransformed size of the node.
     */
    virtual void setContentSize(const CCSize& contentSize);
    /**
     * Returns the untransformed size of the node.
     *
     * @see setContentSize(const CCSize&)
     *
     * @return The untransformed size of the node.
     */
    virtual const CCSize& getContentSize() const;

    
    /**
     * Sets whether the node is visible
     *
     * The default value is true, a node is default to visible
     *
     * @param visible   true if the node is visible, false if the node is hidden.
     */
    virtual void setVisible(bool visible);
    /**
     * Determines if the node is visible
     *
     * @see setVisible(bool)
     *
     * @return true if the node is visible, false if the node is hidden.
     */
    virtual bool isVisible();

	virtual void setOpacity(float opacity) { m_Opacity = opacity; }
	virtual float getOpacity() { return m_Opacity; }

	virtual void setColor(const CCColor3F& c) { m_Color = c; }
	virtual const CCColor3F& getColor() { return m_Color; }

    /**
     * Sets whether the anchor point will be (0,0) when you position this node.
     *
     * This is an internal method, only used by CCLayer and CCScene. Don't call it outside framework.
     * The default value is false, while in CCLayer and CCScene are true
     *
     * @param ignore    true if anchor point will be (0,0) when you position this node
     * @todo This method shoud be renamed as setIgnoreAnchorPointForPosition(bool) or something with "set"
     */
    virtual void ignoreAnchorPointForPosition(bool ignore);
    /**
     * Gets whether the anchor point will be (0,0) when you position this node.
     *
     * @see ignoreAnchorPointForPosition(bool)
     *
     * @return true if the anchor point will be (0,0) when you position this node.
     */
    virtual bool isIgnoreAnchorPointForPosition();
    
    /// @}  end of Setters & Getters for Graphic Properties
    
    
    /// @{
    /// @name Children and Parent
    
    /** 
     * Adds a child to the container with z-order as 0.
     *
     * If the child is added to a 'running' node, then 'onEnter' and 'onEnterTransitionDidFinish' will be called immediately.
     *
     * @param child A child node
     */
    virtual void addChild(CCNode * child);
    /** 
     * Adds a child to the container with a z-order
     *
     * If the child is added to a 'running' node, then 'onEnter' and 'onEnterTransitionDidFinish' will be called immediately.
     *
     * @param child     A child node
     * @param zOrder    Z order for drawing priority. Please refer to setZOrder(int)
     */
    virtual void addChild(CCNode * child, int zOrder);
    /** 
     * Adds a child to the container with z order and tag
     *
     * If the child is added to a 'running' node, then 'onEnter' and 'onEnterTransitionDidFinish' will be called immediately.
     *
     * @param child     A child node
     * @param zOrder    Z order for drawing priority. Please refer to setZOrder(int)
     * @param tag       A interger to identify the node easily. Please refer to setTag(int)
     */
    virtual void addChild(CCNode* child, int zOrder, int tag);
    /**
     * Gets a child from the container with its tag
     *
     * @param tag   An identifier to find the child node.
     *
     * @return a CCNode object whose tag equals to the input parameter
     */
    CCNode * getChildByTag(int tag);
    
    /** 
     * Get the amount of children.
     *
     * @return The amount of children.
     */
    unsigned int getChildrenCount(void) const;
    
    /**
     * Sets the parent node
     *
     * @param parent    A pointer to the parnet node
     */
    virtual void setParent(CCNode* parent);
    /**
     * Returns a pointer to the parent node
     * 
     * @see setParent(CCNode*)
     *
     * @returns A pointer to the parnet node
     */
    virtual CCNode* getParent();
    
    
    ////// REMOVES //////
    
    /** 
     * Removes this node itself from its parent node with a cleanup.
     * If the node orphan, then nothing happens.
     * @see removeFromParentAndCleanup(bool)
     */
    virtual void removeFromParent();
    /** 
     * Removes this node itself from its parent node. 
     * If the node orphan, then nothing happens.
     * @param cleanup   true if all actions and callbacks on this node should be removed, false otherwise.
     */
    virtual void removeFromParentAndCleanup(bool cleanup);
    /** 
     * Removes a child from the container with a cleanup
     *
     * @see removeChild(CCNode, bool)
     *
     * @param child     The child node which will be removed.
     */
    virtual void removeChild(CCNode* child);
    /** 
     * Removes a child from the container. It will also cleanup all running actions depending on the cleanup parameter.
     * 
     * @param child     The child node which will be removed.
     * @param cleanup   true if all running actions and callbacks on the child node will be cleanup, false otherwise.
     */
    virtual void removeChild(CCNode* child, bool cleanup);
    /** 
     * Removes a child from the container by tag value with a cleanup.
     *
     * @see removeChildByTag(int, bool)
     *
     * @param tag       An interger number that identifies a child node
     */
    virtual void removeChildByTag(int tag);
    /** 
     * Removes a child from the container by tag value. It will also cleanup all running actions depending on the cleanup parameter
     * 
     * @param tag       An interger number that identifies a child node
     * @param cleanup   true if all running actions and callbacks on the child node will be cleanup, false otherwise. 
     */
    virtual void removeChildByTag(int tag, bool cleanup);
    /** 
     * Removes all children from the container with a cleanup.
     *
     * @see removeAllChildrenWithCleanup(bool)
     */
    virtual void removeAllChildren();
    /** 
     * Removes all children from the container, and do a cleanup to all running actions depending on the cleanup parameter.
     *
     * @param cleanup   true if all running actions on all children nodes should be cleanup, false oterwise.
     */
    virtual void removeAllChildrenWithCleanup(bool cleanup);
    
    /** 
     * Reorders a child according to a new z value.
     *
     * @param child     An already added child node. It MUST be already added.
     * @param zOrder    Z order for drawing priority. Please refer to setZOrder(int)
     */
    virtual void reorderChild(CCNode * child, int zOrder);
    
    /** 
     * Sorts the children array once before drawing, instead of every time when a child is added or reordered.
     * This appraoch can improves the performance massively.
     * @note Don't call this manually unless a child added needs to be removed in the same frame 
     */
    virtual void sortAllChildren();

    /// @} end of Children and Parent
    
    
    /// @{
    /// @name Tag & User data
    
    /**
     * Returns a tag that is used to identify the node easily.
     *
     * You can set tags to node then identify them easily.
     * @code
     * #define TAG_PLAYER  1
     * #define TAG_MONSTER 2
     * #define TAG_BOSS    3
     * // set tags
     * node1->setTag(TAG_PLAYER);
     * node2->setTag(TAG_MONSTER);
     * node3->setTag(TAG_BOSS);
     * parent->addChild(node1);
     * parent->addChild(node2);
     * parent->addChild(node3);
     * // identify by tags
     * CCNode* node = NULL;
     * CCARRAY_FOREACH(parent->getChildren(), node)
     * {
     *     switch(node->getTag())
     *     {
     *         case TAG_PLAYER:
     *             break;
     *         case TAG_MONSTER:
     *             break;
     *         case TAG_BOSS:
     *             break;
     *     }
     * }
     * @endcode
     *
     * @return A interger that identifies the node.
     */
    virtual int getTag() const;
    /**
     * Changes the tag that is used to identify the node easily.
     *
     * Please refer to getTag for the sample code.
     *
     * @param A interger that indentifies the node.
     */
    virtual void setTag(int nTag);
    
    /**
     * Returns a custom user data pointer
     *
     * You can set everything in UserData pointer, a data block, a structure or an object.
     * 
     * @return A custom user data pointer
     */
    virtual void* getUserData();
    /**
     * Sets a custom user data pointer
     *
     * You can set everything in UserData pointer, a data block, a structure or an object, etc.
     * @warning Don't forget to release the memroy manually, 
     *          especially before you change this data pointer, and before this node is autoreleased.
     *
     * @return A custom user data pointer
     */
    virtual void setUserData(void *pUserData);
    
    /// @} end of Tag & User Data
    
    
    /// @{
    /// @name Event Callbacks
	bool callOnEvent( const CCEvent& event );

		// override me - return true if event handled
	virtual bool onEvent( const CCEvent& event ) { return false; }

    /// @} end of event callbacks.


    /** 
     * Stops all running actions and schedulers
     */
    virtual void cleanup(void);

    /** 
     * Override this method to draw your own node.
     * The following GL states will be enabled by default:
     * - glEnableClientState(GL_VERTEX_ARRAY);
     * - glEnableClientState(GL_COLOR_ARRAY);
     * - glEnableClientState(GL_TEXTURE_COORD_ARRAY);
     * - glEnable(GL_TEXTURE_2D);
     * AND YOU SHOULD NOT DISABLE THEM AFTER DRAWING YOUR NODE
     * But if you enable any other GL state, you should disable it after drawing your node.
     */
    virtual void draw(void);

    /** 
     * Visits this node's children and draw them recursively.
     */
    virtual void visit(void);

    /* 
     * Update method will be called automatically every frame if "scheduleUpdate" is called, and the node is "live"
     */
    virtual void update(float delta);

	void callUpdate(float delta);

    /// @{
    /// @name Transformations

    /** 
     * Returns the matrix that transform the node's (local) space coordinates into the parent's space coordinates.
     * The matrix is in Pixels.
     */
    virtual CCNodeTransform nodeToParentTransform(void);

    /** 
     * Returns the matrix that transform parent's space coordinates to the node's (local) space coordinates.
     * The matrix is in Pixels.
     */
    virtual CCNodeTransform parentToNodeTransform(void);

    /** 
     * Returns the world affine transform matrix. The matrix is in Pixels.
     */
    virtual CCNodeTransform nodeToWorldTransform(void);

    /** 
     * Returns the inverse world affine transform matrix. The matrix is in Pixels.
     */
    virtual CCNodeTransform worldToNodeTransform(void); 

    /// @} end of Transformations
    
    
    /// @{
    /// @name Coordinate Converters
    
    /** 
     * Converts a Point to node (local) space coordinates. The result is in Points.
     */
    CCPoint convertToNodeSpace(const CCPoint& worldPoint);
    
    /** 
     * Converts a Point to world space coordinates. The result is in Points.
     */
    CCPoint convertToWorldSpace(const CCPoint& nodePoint);
    

    /// @} end of Coordinate Converters

private:
    /// helper that reorder a child
    void insertChild(CCNode* child, int z);
    
    /// Removes a child, call child->onExit(), do cleanup, remove it from children array.
    void detachChild(CCNode *child, bool doCleanup);
    
    /// Convert cocos2d coordinates to UI windows coordinate.
    CCPoint convertToWindowSpace(const CCPoint& nodePoint);

protected:
    float m_fScale;                    ///< scaling factor
    CCPoint m_obPosition;               ///< position of the node
    
    CCPoint m_obAnchorPointInPoints;    ///< anchor point in points
    CCPoint m_obAnchorPoint;            ///< anchor point normalized (NOT in points)
    
    CCSize m_obContentSize;             ///< untransformed size of the node

	float m_Opacity;
	CCColor3F m_Color;

    int m_nZOrder;                      ///< z-order value that affects the draw order
	
    std::vector<CCNode*> m_Children;   ///< array of children nodes
    CCNode *m_pParent;                  ///< weak reference to parent node
    
    int m_nTag;                         ///< a tag. Can be any number you assigned just to identify this node
    
    void *m_pUserData;                  ///< A user assingned void pointer, Can be point to any cpp object
    
    bool m_bVisible;                    ///< is this node visible
    
    bool m_bIgnoreAnchorPointForPosition; ///< true if the Anchor Point will be (0,0) when you position the CCNode, false otherwise.
                                          ///< Used by CCLayer and CCScene.
    
    bool m_bReorderChildDirty;          ///< children order dirty flag
};

// end of base_node group
/// @}

NS_CC_END

#endif // __PLATFORM_CCNODE_H__
