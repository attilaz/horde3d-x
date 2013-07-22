// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#include "Scene.h"
#include "Horde3D.h"
#include "Horde3DUtils.h"
#include <math.h>
#include <iomanip>
#include "cocos2d.h"


using namespace std;
using namespace cocos2d;

// Convert from degrees to radians
inline float degToRad( float f ) 
{
	return f * (3.1415926f / 180.0f);
}


Scene::Scene( )
{
	for( unsigned int i = 0; i < 320; ++i )
	{	
		_keys[i] = false;
		_prevKeys[i] = false;
	}

	_x = 15; _y = 3; _z = 20; _rx = -10; _ry = 60; _velocity = 10.0f;
	_curFPS = 30;

	_statMode = 0;
	_freezeMode = 0; _debugViewMode = false; _wireframeMode = false;
	_cam = 0;

	_contentDir = "../Content";

	// Initialize engine
	if( !h3dInit( CCDirector::sharedDirector()->getOpenGLView()->getDevice() ) )
	{	
		h3dutDumpMessages();
		exit(-1);
	}

	// Set options
	h3dSetOption( H3DOptions::LoadTextures, 1 );
	h3dSetOption( H3DOptions::MaxAnisotropy, 4 );
	h3dSetOption( H3DOptions::ShadowMapSize, 2048 );
	h3dSetOption( H3DOptions::FastAnimation, 1 );

	// Add resources
	// Pipelines
	_forwardPipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );
	_deferredPipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/deferred.pipeline.xml", 0 );
	// Overlays
	_fontMatRes = h3dAddResource( H3DResTypes::Material, "overlays/font.material.xml", 0 );
	_panelMatRes = h3dAddResource( H3DResTypes::Material, "overlays/panel.material.xml", 0 );
	_logoMatRes = h3dAddResource( H3DResTypes::Material, "overlays/logo.material.xml", 0 );
	// Shader for deferred shading
	H3DRes lightMatRes = h3dAddResource( H3DResTypes::Material, "materials/light.material.xml", 0 );
	// Environment
	H3DRes envRes = h3dAddResource( H3DResTypes::SceneGraph, "models/platform/platform.scene.xml", 0 );
	// Skybox
	H3DRes skyBoxRes = h3dAddResource( H3DResTypes::SceneGraph, "models/skybox/skybox.scene.xml", 0 );
	
	// Load resources
#ifdef HORDE3D_D3D11
	const char* platformDirectory = "d3d11";
#elif HORDE3D_GL
	const char* platformDirectory = "gl";
#elif HORDE3D_GLES2
	const char* platformDirectory = "gles2";
#endif
	CCDirector::sharedDirector()->loadResourcesFromDisk( _contentDir.c_str(), platformDirectory );

	// Add scene nodes
	// Add camera
	_cam = h3dAddCameraNode( H3DRootNode, "Camera", _forwardPipeRes );
	//h3dSetNodeParamI( _cam, H3DCamera::OccCullingI, 1 );
	// Add environment
	H3DNode env = h3dAddNodes( H3DRootNode, envRes );
	h3dSetNodeTransform( env, 0, 0, 0, 0, 0, 0, 0.23f, 0.23f, 0.23f );
	// Add skybox
	H3DNode sky = h3dAddNodes( H3DRootNode, skyBoxRes );
	h3dSetNodeTransform( sky, 0, 0, 0, 0, 0, 0, 210, 50, 210 );
	h3dSetNodeFlags( sky, H3DNodeFlags::NoCastShadow, true );
	// Add light source
	H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP" );
	h3dSetNodeTransform( light, 0, 20, 50, -30, 0, 0, 1, 1, 1 );
	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 200 );
	h3dSetNodeParamF( light, H3DLight::FovF, 0, 90 );
	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
	h3dSetNodeParamF( light, H3DLight::ShadowSplitLambdaF, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.001f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.7f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.75f );

	_crowdSim = new CrowdSim( _contentDir );
	_crowdSim->init();

	resize( CCDirector::sharedDirector()->getWinSize().width, CCDirector::sharedDirector()->getWinSize().height );
}


void Scene::update( float fDelta )
{
	keyStateHandler();

	for(unsigned int i=0; i<Key::Count; ++i)
		 _prevKeys[i] = _keys[i];

	_curFPS = 1.0f / fDelta;
	
	h3dSetOption( H3DOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
	h3dSetOption( H3DOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f );
	
	if( !_freezeMode )
	{
		_crowdSim->update( _curFPS );
	}
	
	// Set camera parameters
	h3dSetNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 );
	
	// Show stats
	h3dutShowFrameStats( _fontMatRes, _panelMatRes, _statMode );
	if( _statMode > 0 )
	{
		if( h3dGetNodeParamI( _cam, H3DCamera::PipeResI ) == _forwardPipeRes )
			h3dutShowText( "Pipeline: forward", 0.03f, 0.24f, 0.026f, 1, 1, 1, _fontMatRes );
		else
			h3dutShowText( "Pipeline: deferred", 0.03f, 0.24f, 0.026f, 1, 1, 1, _fontMatRes );
	}

	// Show logo
	const float ww = (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportWidthI ) /
	                 (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportHeightI );
	const float ovLogo[] = { ww-0.4f, 0.8f, 0, 1,  ww-0.4f, 1, 0, 0,  ww, 1, 1, 0,  ww, 0.8f, 1, 1 };
	h3dShowOverlays( ovLogo, 4, 1.f, 1.f, 1.f, 1.f, _logoMatRes, 0 );
	
	// Render scene
	h3dRender( _cam );

	// Finish rendering of frame
	h3dFinalizeFrame();

	// Remove all overlays
	h3dClearOverlays();

	// Write all messages to log file
	h3dutDumpMessages();
}


bool Scene::onEvent( const CCEvent& e  )
{
	switch(e.Type)
	{
	case CCEvent::Quit:
		release();
		return true;
	case CCEvent::TouchBegan:
		_pointerDown = true;
		_pointerX = e.Touch.X; _pointerY = e.Touch.Y;
	break;
	case CCEvent::TouchEnded:
	case CCEvent::TouchCancelled:
		_pointerDown = false;
	break;
	case CCEvent::TouchMoved:
		if (_pointerDown)
		{
			mouseMoveEvent( e.Touch.X - _pointerX, e.Touch.Y - _pointerY);
			_pointerX = e.Touch.X; _pointerY = e.Touch.Y;
		}
	break;
	case CCEvent::KeyPressed:
		_keys[e.Key.Code] = true;
	break;
	case CCEvent::KeyReleased:
		_keys[e.Key.Code] = false;
	break;
	}

	return false;
}

void Scene::release()
{
	delete _crowdSim; _crowdSim = 0x0;
	
	// Release engine
	h3dRelease();
}


void Scene::resize( int width, int height )
{
	// Resize viewport
	h3dSetNodeParamI( _cam, H3DCamera::ViewportXI, 0 );
	h3dSetNodeParamI( _cam, H3DCamera::ViewportYI, 0 );
	h3dSetNodeParamI( _cam, H3DCamera::ViewportWidthI, width );
	h3dSetNodeParamI( _cam, H3DCamera::ViewportHeightI, height );
	
	// Set virtual camera parameters
	h3dSetupCameraView( _cam, 45.0f, (float)width / height, 0.1f, 1000.0f );
	h3dResizePipelineBuffers( _deferredPipeRes, width, height );
	h3dResizePipelineBuffers( _forwardPipeRes, width, height );
}


void Scene::keyStateHandler()
{
	// ----------------
	// Key-press events
	// ----------------
	if( _keys[Key::Space] && !_prevKeys[Key::Space] )  // Space
	{
		if( ++_freezeMode == 3 ) _freezeMode = 0;
	}
	
	if( _keys[Key::F3] && !_prevKeys[Key::F3] )  // F3
	{
		if( h3dGetNodeParamI( _cam, H3DCamera::PipeResI ) == _forwardPipeRes )
			h3dSetNodeParamI( _cam, H3DCamera::PipeResI, _deferredPipeRes );
		else
			h3dSetNodeParamI( _cam, H3DCamera::PipeResI, _forwardPipeRes );
	}
	
	if( _keys[Key::F7] && !_prevKeys[Key::F7] )  // F7
		_debugViewMode = !_debugViewMode;

	if( _keys[Key::F8] && !_prevKeys[Key::F8] )  // F8
		_wireframeMode = !_wireframeMode;
	
	if( _keys[Key::F6] && !_prevKeys[Key::F6] )  // F6
	{
		_statMode += 1;
		if( _statMode > H3DUTMaxStatMode ) _statMode = 0;
	}

	// --------------
	// Key-down state
	// --------------
	if( _freezeMode != 2 )
	{
		float curVel = _velocity / _curFPS;
		
		if( _keys[Key::LShift] ) curVel *= 5;	// LShift
		
		if( _keys[Key::W] )
		{
			// Move forward
			_x -= sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
			_y -= sinf( -degToRad( _rx ) ) * curVel;
			_z -= cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		}
		if( _keys[Key::S] )
		{
			// Move backward
			_x += sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
			_y += sinf( -degToRad( _rx ) ) * curVel;
			_z += cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		}
		if( _keys[Key::A] )
		{
			// Strafe left
			_x += sinf( degToRad( _ry - 90) ) * curVel;
			_z += cosf( degToRad( _ry - 90 ) ) * curVel;
		}
		if( _keys[Key::D] )
		{
			// Strafe right
			_x += sinf( degToRad( _ry + 90 ) ) * curVel;
			_z += cosf( degToRad( _ry + 90 ) ) * curVel;
		}
	}
}


void Scene::mouseMoveEvent( float dX, float dY )
{
	if( _freezeMode == 2 ) return;
	
	// Look left/right
	_ry -= dX / 100.0f * 30.0f;
	
	// Loop up/down but only in a limited range
	_rx += dY / 100.0f * 30.0f;
	if( _rx > 90 ) _rx = 90; 
	if( _rx < -90 ) _rx = -90;
}
