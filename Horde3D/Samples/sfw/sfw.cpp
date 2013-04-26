#include "sfw.h"
#include <queue>
#include "Horde3D.h"
#include <string>
#include <fstream>

using namespace std;

static	std::queue<sfwEvent> events;

extern "C" bool sfwGetEvent(sfwEvent* e)	//query event, returns false in no more event
{
    // Pop first event of queue, if not empty
    if (!events.empty())
    {
        *e = events.front();
        events.pop();
        return true;
    }

	e->type = sfwEventType::None;
    return false;
}

extern "C" void sfwPushEvent(sfwEvent* e)	//adds event, used by platform implementations
{
	events.push(*e);
}

string cleanPath( string path )
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

void splitPath(const std::string& full, std::string& path, std::string& nameWithExtension)
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

extern "C" void sfwLoadResourcesFromDisk( const char *contentDir, const char* platformSubDir )
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
