#pragma once


//ofxWMFVideoPlayer addon written by Philippe Laulheret for Second Story (secondstory.com)
//Based upon Windows SDK samples
//MIT Licensing


#include "ofMain.h"
#include "ofxWMFVideoPlayerUtils.h"

#include "EVRPresenter.h"



class ofxWMFVideoPlayer;


class CPlayer;
class ofxWMFVideoPlayer {

	private:
		static int  _instanceCount;
		
		
		HWND		_hwndPlayer;
		
		BOOL bRepaintClient;
		
		
		int _width;
		int _height;


		bool _waitForLoadedToPlay;
		bool _isLooping;
		bool _wantToSetVolume;
		float _currentVolume;

		bool _sharedTextureCreated;
		
		ofTexture _tex;
	
		BOOL InitInstance();

		
		void                OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr);

		float _frameRate;



	public:
	CPlayer*	_player;

	int _id;

	
	ofxWMFVideoPlayer();
	 ~ofxWMFVideoPlayer();

	 bool				loadMovie(string name);
	 //bool 				loadMovie(string name_left, string name_right) ;
	 void				close();
	 void				update();
	
	 void				play();
	 void				stop();		
	 void				pause();

	 float				getPosition();
	 float				getDuration();
	 float				getFrameRate();

	 void				setPosition(float pos);

	 void				setVolume(float vol);
	 float				getVolume();

	 float				getHeight();
	 float				getWidth();

	 bool				isPlaying(); 
	 bool				isStopped();
	 bool				isPaused();

	 void				setLoop(bool isLooping);
	 bool				isLooping() { return _isLooping; }



	




	void draw(int x, int y , int w, int h);
	void draw(int x, int y) { draw(x,y,getWidth(),getHeight()); }


	HWND getHandle() { return _hwndPlayer;}
	LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	static void forceExit();


};