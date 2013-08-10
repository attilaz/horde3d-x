horde3d-x
=========
Horde3D is a small open source 3D rendering engine.

This branch is still WORK-IN-PROGRESS and anything is subject to change!
The goal of this branch is to add GLES2 and D3D11 support with samples on multiple platforms (iOS, android, nacl, emscripten, win8, winrt,...).
GLES2 renderer is fairly stable, already used in commercial products.

Copyright (C) 2006-2013 Nicolas Schulz and the Horde3D Team

http://www.horde3d.org

The complete SDK is licensed under the terms of the Eclipse Public License (EPL).

Horde3D requires an OpenGL 2.0 compatible graphics card with the latest drivers.
A GeForce 6 or Radeon X1000 series card is the minimum requirement to run the samples.

The source code of the engine and tools is included in the SDK. It has the following dependencies:

	- RapidXml
		http://rapidxml.sourceforge.net
	- stbi by Sean Barrett
		http://nothings.org
	- GLFW for window management in samples
		http://glfw.sourceforge.net
		
These libraries are included directly as code in the SDK.

