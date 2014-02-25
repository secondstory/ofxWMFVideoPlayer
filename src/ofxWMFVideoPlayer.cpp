//ofxWMFVideoPlayer addon written by Philippe Laulheret for Second Story (secondstory.com)
//MIT Licensing


#include "ofxWMFVideoPlayerUtils.h"
#include "ofxWMFVideoPlayer.h"



typedef std::pair<HWND,ofxWMFVideoPlayer*> PlayerItem;
list<PlayerItem> g_WMFVideoPlayers;





LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
// Message handlers


ofxWMFVideoPlayer* findPlayers(HWND hwnd)
{
	for each (PlayerItem e in g_WMFVideoPlayers)
	{
		if (e.first == hwnd) return e.second;
	}

	return NULL;
}


int  ofxWMFVideoPlayer::_instanceCount=0;


ofxWMFVideoPlayer::ofxWMFVideoPlayer() : _player(NULL)
{
	
	if (_instanceCount ==0)  {
		if (!ofIsGLProgrammableRenderer()){
			if(wglewIsSupported("WGL_NV_DX_interop")){
				ofLogVerbose("ofxWMFVideoPlayer") << "WGL_NV_DX_interop supported";
			}else{
				ofLogError("ofxWMFVideoPlayer") << "WGL_NV_DX_interop not supported. Upgrade your graphc drivers and try again.";
				return;
			}
		}


		HRESULT hr = MFStartup(MF_VERSION);
	  if (!SUCCEEDED(hr))
    {
		ofLog(OF_LOG_ERROR, "ofxWMFVideoPlayer: Error while loading MF");
    }
	}

	_id = _instanceCount;
	_instanceCount++;
	this->InitInstance();
	

	_waitForLoadedToPlay = false;
	_sharedTextureCreated = false;
	
	
}
	 
	 
ofxWMFVideoPlayer::~ofxWMFVideoPlayer() {
	if (_player)
    {
		_player->Shutdown();
		//if (_sharedTextureCreated) _player->m_pEVRPresenter->releaseSharedTexture();
        SafeRelease(&_player);
    }

	cout << "Player " << _id << " Terminated" << endl;
	_instanceCount--;
	if (_instanceCount == 0) {
		 MFShutdown();

		 cout << "Shutting down MF" << endl;
	}

}

void ofxWMFVideoPlayer::forceExit()
{
	if (_instanceCount != 0) 
	{
		cout << "Shutting down MF some ofxWMFVideoPlayer remains" << endl;
		MFShutdown();
	}
		

		
	
}

 bool	ofxWMFVideoPlayer::	loadMovie(string name) 
 {
	 if (!_player) { 
		ofLogError("ofxWMFVideoPlayer") << "Player not created. Can't open the movie.";
		 return false;
	 }
	 	DWORD fileAttr = GetFileAttributesA(ofToDataPath(name).c_str());
		if (fileAttr == INVALID_FILE_ATTRIBUTES) {
		stringstream s;
		s << "The video file '" << name << "'is missing.";
		ofLog(OF_LOG_ERROR,"ofxWMFVideoPlayer:" + s.str());
		return false;
	}

		//cout << "Videoplayer[" << _id << "] loading " << name << endl;

	HRESULT hr = S_OK;
		string s = ofToDataPath(name);
		std::wstring w(s.length(), L' ');
		std::copy(s.begin(), s.end(), w.begin());

		
	hr = _player->OpenURL( w.c_str());

	
	 


	
	 if (!_sharedTextureCreated)
	 {

		 _width = _player->getWidth();
		 _height = _player->getHeight();

		 _tex.allocate(_width,_height,GL_RGBA,true);

		_player->m_pEVRPresenter->createSharedTexture(_width, _height,_tex.texData.textureID);
		_sharedTextureCreated = true;
	 }
	 else 
	 {
		 if ((_width != _player->getWidth()) || (_height != _player->getHeight()))
		 {

			 _player->m_pEVRPresenter->releaseSharedTexture();

			 _width = _player->getWidth();
			 _height = _player->getHeight();

			 _tex.allocate(_width,_height,GL_RGBA,true);
			 _player->m_pEVRPresenter->createSharedTexture(_width, _height,_tex.texData.textureID);

		 }
		 
	 }
	 _waitForLoadedToPlay = false;
	 return false;

	
 }



 void ofxWMFVideoPlayer::draw(int x, int y , int w, int h) {


	 _player->m_pEVRPresenter->lockSharedTexture();
	 _tex.draw(x,y,w,h);
	  _player->m_pEVRPresenter->unlockSharedTexture();

	 

 }


bool  ofxWMFVideoPlayer:: isPlaying() {
	return _player->GetState() == Started;
 }
bool  ofxWMFVideoPlayer:: isStopped() {
	return (_player->GetState() == Stopped || _player->GetState() == Paused);
 }

bool  ofxWMFVideoPlayer:: isPaused() 
{
	return _player->GetState() == Paused;
}




 void	ofxWMFVideoPlayer::	close() {
	 _player->Shutdown();

}
void	ofxWMFVideoPlayer::	update() {
	if (!_player) return;
	if ((_waitForLoadedToPlay) && _player->GetState() == Paused)
	{
		_waitForLoadedToPlay=false;
		_player->Play();
		
	}
	return;
 }



	
void	ofxWMFVideoPlayer::	play() 
{

	if (!_player) return;
	if (_player->GetState()  == OpenPending) _waitForLoadedToPlay = true;
	_player->Play();
}

void	ofxWMFVideoPlayer::	stop() 
{
	_player->Stop();
}

void	ofxWMFVideoPlayer::	pause() 
{
	_player->Pause();
}


float 			ofxWMFVideoPlayer::	getPosition() {
	return _player->getPosition();
}

float 			ofxWMFVideoPlayer::	getDuration() {
	return _player->getDuration();
}

void ofxWMFVideoPlayer::setPosition(float pos)
{
	_player->setPosition(pos);
}

float	ofxWMFVideoPlayer::getHeight() { return _player->getHeight(); }
float	ofxWMFVideoPlayer::getWidth() { return _player->getWidth(); }

void  ofxWMFVideoPlayer::setLoop(bool isLooping) { _isLooping = isLooping; _player->setLooping(isLooping); }



//-----------------------------------
// Prvate Functions
//-----------------------------------



// Handler for Media Session events.
void ofxWMFVideoPlayer::OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr)
{
    HRESULT hr = _player->HandleEvent(pUnkPtr);
    if (FAILED(hr))
    {
        ofLogError("ofxWMFVideoPlayer", "An error occurred.");
    }
 }



LRESULT CALLBACK WndProcDummy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{


    switch (message)
    {

    case WM_CREATE:
		{
			return DefWindowProc(hwnd, message, wParam, lParam);
			
		}
    default:
		{
		ofxWMFVideoPlayer*   myPlayer = findPlayers(hwnd);
		 if (!myPlayer) return DefWindowProc(hwnd, message, wParam, lParam);
		 return myPlayer->WndProc (hwnd, message, wParam, lParam);
		}
    }
    return 0;
}



LRESULT  ofxWMFVideoPlayer::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_DESTROY:
        PostQuitMessage(0);
        break;


    case WM_APP_PLAYER_EVENT:
        OnPlayerEvent(hwnd, wParam);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}





//  Create the application window.
BOOL ofxWMFVideoPlayer::InitInstance()
{
	PCWSTR szWindowClass = L"MFBASICPLAYBACK" ;
    HWND hwnd;
    WNDCLASSEX wcex;

 //   g_hInstance = hInst; // Store the instance handle.

    // Register the window class.
    ZeroMemory(&wcex, sizeof(WNDCLASSEX));
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW  ;

    wcex.lpfnWndProc    =  WndProcDummy;
  //  wcex.hInstance      = hInst;
	wcex.hbrBackground  = (HBRUSH)(BLACK_BRUSH);
   // wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_MFPLAYBACK);
    wcex.lpszClassName  = szWindowClass;

    if (RegisterClassEx(&wcex) == 0)
    {
       // return FALSE;
    }


    // Create the application window.
    hwnd = CreateWindow(szWindowClass, L"", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, NULL, NULL);

    if (hwnd == 0)
    {
        return FALSE;
    }


	g_WMFVideoPlayers.push_back(std::pair<HWND,ofxWMFVideoPlayer*>(hwnd,this));
	HRESULT hr = CPlayer::CreateInstance(hwnd, hwnd, &_player); 



	LONG style2 = ::GetWindowLong(hwnd, GWL_STYLE);  
    style2 &= ~WS_DLGFRAME;
    style2 &= ~WS_CAPTION; 
    style2 &= ~WS_BORDER; 
    style2 &= WS_POPUP;
    LONG exstyle2 = ::GetWindowLong(hwnd, GWL_EXSTYLE);  
    exstyle2 &= ~WS_EX_DLGMODALFRAME;  
    ::SetWindowLong(hwnd, GWL_STYLE, style2);  
    ::SetWindowLong(hwnd, GWL_EXSTYLE, exstyle2);  



	_hwndPlayer = hwnd;



    UpdateWindow(hwnd);
	

    return TRUE;
}

