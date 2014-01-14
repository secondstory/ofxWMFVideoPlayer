ofxWMFVideoPlayer
=================

ofxWMFVideoPlayer is an addon for openFrameworks that uses the Windows Media Foundation (WMF) API to read videos. 
It's an interesting videoplayback solution as it uses Windows H264 decoder (therefore avoiding any patent issue for using third party decoder) and was developed with hardware acceleration in mind.

##Purpose of this document

This documents is a technical description of the work done to implement the player, as it might be useful for other people to maintain it, extend it, or simply transpose this work to a similar problem. It also presents the Windows Media Foundation architecture, and various findings that were made during the development (as unfortunately Microsoft documentation can be lacking of useful details here and there)

##Licensing

This addon was developped  by Philippe Laulheret for [Second Story](http://www.secondstory.com) and was heavily based upon Microsoft SDK samples. This addon is released under MIT license so feel free to use it as you wish. Feel free to fork this on github and share issues with us.

##General architecture

Microsoft is kind enough to deliver their SDK with samples, so instead of having to re-invent the wheel, a good deal of the development was based upon the sdk documentation that can be found [there](http://msdn.microsoft.com/en-us/library/windows/desktop/ms703190(v=vs.85).aspx). Which basically consists of creating a basic video player (the microsfot way) and then interface it with openFrameworks to follow (mostly) the ofVideoPlayer template.

##The quest for hardware acceleration

Reading a H264 video on a powerful machine is most of the time not a problem. But when your machine is not that powerful (like if you are using a tiny computer that are meant to be a media platform to plug on your tv) or when you want to play a couple of Ultra-HD videos smoothly, this is a totally different matter. 
The way players in openFrameworks usually works is CPU intensive : they use a third party library (quicktime, gstreamer, vlc) which are usually decoding the video on the CPU side and then returns a gigantic array of pixels, that in turn must be uploaded back to the graphic card. This is highly inefficient as most of the hard work can (and should) be done on the graphic card.
When you think about it, it's not that often that you actually need to access the pixel values of a video on the CPU side; most of the time if you need anything it's more the texture of the video on which you could apply some shader transform.

###First try : using an overlapped window
Our first shot at playing accelerated video was dead simple. For the specific problem we had in mind, nothing more than blending in and out the video was needed. As a consequence, instead of trying to solve the hard problem of hardware acceleration within openGL, we just created a D3D frameless window in that we would hide and show using Windows API. Then we would create a regular video player using the SDK sample, and overlap the whole thing when needed. We won't give much more details about this solution as it's globally unsatisfactory but if ever the need arise to overlap a second window on top of oF, here's the code used to alpha blend it : 

````
void ofxWMFVideoPlayer::    setTransparency(float alpha) {
    
	if (alpha == 255) {
		if (!_isTranparencyEnabled) return;
		_isTranparencyEnabled = false;
		// Remove WS_EX_LAYERED from this window styles
		SetWindowLong(_hwndPlayer, GWL_EXSTYLE,
        GetWindowLong(_hwndPlayer, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		return;

	}

	if (!_isTranparencyEnabled) {
		_isTranparencyEnabled = true;
		// Set WS_EX_LAYERED on this window 
		SetWindowLong(_hwndPlayer, GWL_EXSTYLE,
        GetWindowLong(_hwndPlayer, GWL_EXSTYLE) | WS_EX_LAYERED);
	}
	SetLayeredWindowAttributes(_hwndPlayer, 0, alpha, LWA_ALPHA);
	return;
}
```` 

###The graal of HW acceleration : WGL_NV_DX_interop

Instead of dumbly overlapping a window on top of our application, getting the texture inside of openGL is a much more interesting goal to achieve. Luckily for us, NVIDIA worked on it, and created an openGL extension that does exactly what we are looking for. With `WGL_NV_DX_interop` you can share Direct3D object with openGL and use rendered texture from one into the other.
For a simple usage of this extension with openFrameworks, you can refer to our [github](https://github.com/secondstory/ofDxSharedTextureExample).

The idea here is to create a custom rendering target for the media foundation: a direct3D accelerated surface we control. Then, through the openGL extension we get back the texture into openFrameworks, and display seamlessly our video.
More details about WMF in the next part.

###Out in the wild, ANGLE

If you wonder how other people deal with this kind of issue, an other noteworthy approach is called ANGLE. In a nutshell, this is a OpenGL ES 2.0 implementation... running on DirectX. Basically all your openGL calls are turned into Direct3D API calls and the framework offers interop functionality as well. This is how chromium renders efficiently webGL content on windows (actually all the rendering of Chrome is done using this)

##More about Windows Media Foundation (WMF)

###Nodes and Topology

In the WMF a media file is represented as a "Source" that must be connected to one or multiple sinks (audio sink, video sink, ...). A topology is the ensemble of nodes and the connection between them. It's meant to describe the path followed by the different media streams you have.
Something pretty magic about the Topologies in Media Foundation is that you don't have to fully describe you topology to make it work, you can ask windows to complete the missing part for you (intermediate transformation, decoders, ...) and voila! you have a working video playback.

You can read more about Topologies [there](http://msdn.microsoft.com/en-us/library/windows/desktop/aa369306(v=vs.85).aspx)

###Tools

If you need to struggle with topologies, a couple of tools may become handy :

- Topoedit: To visualize a Topology (or check if windows can read your file) the windows sdk comes with a sample called TopoEdit that enables you to visually create a topology and see if it resolves properly. It's a great tool to test what works and what doesn't.

- MFTrace : runs a debug trace of your application, storing in a log file all Media Foundation related messages

###Implementing the texture sharing mechanism

The core idea here is that we're creating a D3D window that we won't show, using its message pump to propagate media foundation messages and that runs an instance of the sample video player from the SDK that we have partially modified.

In order to use the extensions we previously mentioned, we needed to get a closer access to the presentation cycle of the media foundation. For that purpose we created a custom EVR (the video sink) based upon the samples in the SDK. The relevant code is in *src\presenter\PresenterEngine.cpp* and is a rough adaptation of *Samples\multimedia\mediafoundation\evrpresenter* from the Windows SDK samples.
We are basically adding a couple of functions to create a shared texture between D3D and openGL, and on every presentation, we copy the back buffer that's going to be presented to our own surface.

````
HRESULT D3DPresentEngine::PresentSwapChain(IDirect3DSwapChain9* pSwapChain, IDirect3DSurface9* pSurface)
{
<...>
	IDirect3DSurface9 *surface;
	pSwapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&surface);
    if (m_pDevice->StretchRect(surface,NULL,d3d_shared_surface,NULL,D3DTEXF_NONE) != D3D_OK)
	{
		printf("ofxWMFVideoPlayer: Error while copying texture to gl context \n");
	}
	SAFE_RELEASE(surface);
	
<...>
}

````

This might not be the most optimized approach (as we are still presenting the texture to the D3D surface) but this solution was already performing much better than the other solution available we had and so we limited the scope of our work to this approach. 

#### Note on using WGL_NV_DX_interop

Among other things, one can share `IDirect3DSurface9` and `IDirect3DTexture9` with openGL. It appears after some experimentation that sharing a texture is more reliable than sharing a surface. Here's how we reached this conclusion.
Internally to Direct3D9 you can create a shared handle to a ressouce while creating it (useful to share resources across multiple devices/versions of D3D). This might silently fail (the shared handle will be null, but the resource will be successfully created). The failure seems to be computer specific (it's either graphic card related or linked to the updates installed on the computer).  You don't need to create a shared handle to use the WGL_NV_DX_interop extension, but empirically it appears that when the creation of the shared handle fails (/would have failed), the registering of an openGL-direct3D binding fails as well. On the other hand, we saw failure of registering `IDirect3DSurface9` but no failure for `IDirect3DTexture9`. This explains our choice of sharing a texture rather than a surface.

###Bonus (experimental) feature : playing multiple video in sync

We faced an unexpected issue while working on the project for which we developed the video player : Some videos we wanted to play had an unusual format (3840x1080  and on windows 7 can only decode 1920x1080 H264 video), so we tried to split the video in two, and Framesync them. If you Google "windows media foundation multiple source" you can land on the page of the Sequencer Source, with which  "You can use it to create playlists, or to play streams from multiple sources simultaneously." The latter seemed perfect for us, but unfortunately is (apparently) never explained. 
The surprising fact is if you add two media source inside Topoedit, and ask it to resolve the Topology, it will do it successfully. It's because the software is actually using the sequencer source to do the work.

The idea here is roughly : you create your fancy topology with all the sources you want, feed it into the Source Sequencer, ask the Source Sequencer to give you back an amended topology, and voila!

````
	MFSequencerElementId NewID;
	hr = m_pSequencerSource->AppendTopology(pTopology, SequencerTopologyFlags_Last, &NewID) ;
	_previousTopoID = NewID;
	hr = m_pSequencerSource->QueryInterface(IID_IMFMediaSource, (void**) &spSrc) ;
	hr = spSrc->CreatePresentationDescriptor(&spPD) ;
	hr = m_pSequencerSource->QueryInterface(IID_IMFMediaSourceTopologyProvider, (void**) &spSrcTopoProvider) ;
	SafeRelease(&pTopology);
	hr = spSrcTopoProvider->GetMediaSourceTopology(spPD, &pTopology) ;
	//Now that we're done, we set the topolgy as it should be....
	hr = m_pSession->SetTopology(0, pTopology);

````

The media session will make sure then that your videos don't get (too much) out of sync. Unfortunately it's not a perfect frame sync, and because of different compression of the videos, little delay in the processing... once in a while you'll notice the two videos are one or two frames off (even if later on they catch up again). 

A solution to that problem was to swtich to WMV which offer a better range of format (windows 7 can decode up to Ultra-HD wmv files). 

Apparently another option would be to use the `MFCreateAggregateSource` to aggregate multiple sources. Might be handy the time you need to run a 8K video.



##What's next?

At the moment of writing there are still pending issues with using the interop extension on various hardware. To be able to support more graphic cards / setups, implementing a CPU-side pipeline would be a good fallback.
The [Sample grabber sink](http://msdn.microsoft.com/en-us/library/windows/desktop/hh184779(v=vs.85).aspx) is a  good candidate for this solution. However you need to remember that using this sink will break the acceleration chain and as a consequence, will be more CPU-intensive than the current implementation. However it's no worse than other approach using quicktime, gstreamer, vlc,... 















