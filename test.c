//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "os_generic.h"
#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <byteswap.h>
#include <errno.h>
#include <fcntl.h>
#include "CNFGAndroid.h"

//#define CNFA_IMPLEMENTATION
#define CNFG_IMPLEMENTATION
#define CNFG3D

//#include "cnfa/CNFA.h"
#include "CNFG.h"

unsigned frames = 0;
unsigned long iframeno = 0;

void AndroidDisplayKeyboard(int pShow);

int lastbuttonx = 0;
int lastbuttony = 0;
int lastmotionx = 0;
int lastmotiony = 0;
int lastbid = 0;
int lastmask = 0;
int lastkey, lastkeydown;

static int keyboard_up;
uint8_t buttonstate[8];

void HandleKey( int keycode, int bDown )
{
	lastkey = keycode;
	lastkeydown = bDown;
	if( keycode == 10 && !bDown ) { keyboard_up = 0; 
        AndroidDisplayKeyboard( keyboard_up );  
    }

	if( keycode == 4 ) { AndroidSendToBack( 1 ); } //Handle Physical Back Button.
}

void HandleButton( int x, int y, int button, int bDown )
{
	buttonstate[button] = bDown;
	lastbid = button;
	lastbuttonx = x;
	lastbuttony = y;

	if( bDown ) { keyboard_up = !keyboard_up; 
        /* AndroidDisplayKeyboard( keyboard_up ); */ 
    }
}

void HandleMotion(int x, int y, int mask) {}

#define HMX 162
#define HMY 162
short screenx, screeny;

extern struct android_app * gapp;


int HandleDestroy()
{
	printf( "Destroying\n" );
	return 0;
}

volatile int suspended;

void HandleSuspend()
{
	suspended = 1;
}

void HandleResume()
{
	suspended = 0;
}


void HandleThisWindowTermination()
{
	suspended = 1;
}


int main( int argc, char ** argv )
{
	double ThisTime;
	double LastFPSTime = OGGetAbsoluteTime();

	CNFGBGColor = 0x000040ff;
	CNFGSetupFullscreen( "Test Bench", 0 );
	
	HandleWindowTermination = HandleThisWindowTermination;

	const char * assettext = "Not Found";
	AAsset * file = AAssetManager_open( gapp->activity->assetManager, "asset.txt", AASSET_MODE_BUFFER );
	if( file )
	{
		size_t fileLength = AAsset_getLength(file);
		char * temp = (char*)malloc( fileLength + 1);
		memcpy( temp, AAsset_getBuffer( file ), fileLength );
		temp[fileLength] = 0;
		assettext = temp;
	}

	while(1)
	{
		int i;
		iframeno++;

		CNFGHandleInput();

		if( suspended ) { usleep(50000); continue; }

		CNFGClearFrame();
		CNFGColor( 0xFFFFFFFF );
		CNFGGetDimensions( &screenx, &screeny );

		// Mesh in background
		CNFGSetLineWidth( 9 );
		CNFGPenX = 0; CNFGPenY = 400;
		CNFGColor( 0xffffffff );
		CNFGDrawText( assettext, 15 );
		CNFGFlushRender();

		CNFGPenX = 0; CNFGPenY = 480;
		char st[50];
		sprintf( st, "%dx%d %d %d %d %d %d %d\n%d %d\n\nNice", screenx, screeny, lastbuttonx, lastbuttony, lastmotionx, lastmotiony, lastkey, lastkeydown, lastbid, lastmask );
		CNFGDrawText( st, 10 );
		CNFGSetLineWidth( 2 );

		// Square behind text
		CNFGColor( 0x303030ff );
		CNFGTackRectangle( 600, 0, 950, 350);

		CNFGPenX = 10; CNFGPenY = 10;

		// Text
		CNFGColor( 0xffffffff );
		for( i = 0; i < 1; i++ )
		{
			int c;
			char tw[2] = { 0, 0 };
			for( c = 0; c < 256; c++ )
			{
				tw[0] = c;

				CNFGPenX = ( c % 16 ) * 20+606;
				CNFGPenY = ( c / 16 ) * 20+5;
				CNFGDrawText( tw, 4 );
			}
		}


		frames++;
		//On Android, CNFGSwapBuffers must be called, and CNFGUpdateScreenWithBitmap does not have an implied framebuffer swap.
		CNFGSwapBuffers();

		ThisTime = OGGetAbsoluteTime();
		if( ThisTime > LastFPSTime + 1 )
		{
			printf( "FPS: %d\n", frames );
			frames = 0;
			LastFPSTime+=1;
		}

	}

	return(0);
}

