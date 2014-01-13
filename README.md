ofxWMFVideoPlayer
========================

This addons is an accelerated video player using the Windows Media Foundation API. 
It was developped by Philippe Laulheret for [Second Story] (http://www.secondstory.com) and is released under MIT licessing. See license.md for more details. 
For a more comprehensive/technical description of the work, please refer to the file TechnicalDescription.md

This addons is meant to work with openFrameworks 0.8 and superior, on windows Vista and following.
Previous version of oF should work as well but won't be maintained.

#Known issue

At the time of writing the texture sharing works, or fails, depending on the machine it runs on. This behaviour might be graphic card related issue or some misuse of Direct3D. This problen is being investigated. The faulty code happens on the call to `wglDXRegisterObjectNV` in PresentEngine.h. The handle this function returns should never be null but depending on the machine, the call will fail or succeed.

#Prerequisites 

This addons uses the WGL_NV_DX_interop extension, in order to use it you need to upgrade the GLEW library of your openFrameworks folder :
Prior to compiling this example you need to replace the GLEW headers and libraries from your of\libs foler by the one present in the lib folder of this repository.

On top of that this addon is built against the Direct X SDK of June 2010. You'll need to install it to have the headers required for compiling the example. At the time of writing, you can download it on the [Microsoft website](http://www.microsoft.com/en-us/download/details.aspx?id=6812)


#Using the example

Make sure you have updated the GLEW library, your graphic driver are up to date.
Copy (and rename) a mp4 file as "test.mp4" into the data folder and run the example.


#Compatibility 

The texture sahring we are using is based upon an NVIDIA extension but AMD/ATI cards claims they support it was well, even though sometimes it gets more finicky.
Feedbacks on working/not working cards and drivers are more than welcomed.




