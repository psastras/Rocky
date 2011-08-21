#define _WIN32_WINNT 0x0500

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "glcommon.h"
#include "glengine.h"
#include "gltextureloader.h"
#include <string.h>
#include <sstream>
#include <iostream>
#include "common.h"
#include "keyboardcontroller.h"

#ifdef _WIN32

#else
    #include <X11/X.h>
    #include <X11/keysym.h>
    #include <X11/keysymdef.h>
    #include <X11/Xlib.h>
    #include <X11/cursorfont.h>
    #include <X11/Xutil.h>
    #include <GL/glx.h>
    #include <sys/resource.h>
#endif
GLTextureLoader *GLTextureLoader::s_instance = 0;
GLFramebufferManager *GLFramebufferManager::s_instance = 0;
GLEngine *pEngine = 0;
KeyboardController *pKeyController = new KeyboardController();
#ifdef _WIN32

LONG WINAPI
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT ps;
    switch(uMsg) {
    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    case WM_MOUSEMOVE:
	return 0;
    case WM_SIZE:
	//glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
	
	pEngine->resize(LOWORD(lParam), HIWORD(lParam));
	//std::cout << "(w , h) : " << LOWORD(lParam) << " x " << HIWORD(lParam) << std::endl;
        return 0;
    case WM_KEYUP:
	pKeyController->keyReleaseEvent(wParam);
	return 0;
    case WM_KEYDOWN:
	pKeyController->keyPressEvent(wParam);
	//std::cout << "key press: " << wParam << std::endl;
	return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND CreateOpenGLWindow(char* title, int x, int y, int width, int height,
                   BYTE type, DWORD flags) {


    int         pf;
    HDC         hDC;
    HWND        hWnd;
    WNDCLASS    wc;
    PIXELFORMATDESCRIPTOR pfd;
    static HINSTANCE hInstance = 0;

    /* only register the window class once - use hInstance as a flag. */
    if (!hInstance) {
        hInstance = GetModuleHandle(NULL);
        wc.style         = CS_OWNDC;
        wc.lpfnWndProc   = (WNDPROC)WindowProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
	wc.lpszClassName = "OpenGL";

        if (!RegisterClass(&wc)) {
            MessageBox(NULL, "RegisterClass() failed:  "
                       "Cannot register window class.", "Error", MB_OK);
            return NULL;
        }
    }

    hWnd = CreateWindow("OpenGL", title, WS_OVERLAPPEDWINDOW |
                        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        x, y, width, height, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) {
        MessageBox(NULL, "CreateWindow() failed:  Cannot create a window.",
                   "Error", MB_OK);
        return NULL;
    }

    hDC = GetDC(hWnd);

    /* there is no guarantee that the contents of the stack that become
       the pfd are zeroed, therefore _make sure_ to clear these bits. */
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
    pfd.iPixelType   = type;
    pfd.cColorBits   = 32;

    pf = ChoosePixelFormat(hDC, &pfd);
    if (pf == 0) {
        MessageBox(NULL, "ChoosePixelFormat() failed:  "
                   "Cannot find a suitable pixel format.", "Error", MB_OK);
        return 0;
    }

    if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
        MessageBox(NULL, "SetPixelFormat() failed:  "
                   "Cannot set format specified.", "Error", MB_OK);
        return 0;
    }

    DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    ReleaseDC(hWnd, hDC);

    return hWnd;
}

double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter() {
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
	std::cout << "QueryPerformanceFrequency failed!\n";
    PCFreq = double(li.QuadPart)/1000.0;
    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

double GetCounter() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}
#define GLEW_STATIC
int main(int argc, char *argv[]) {
    WindowProperties properties = {1280, 720};
    HDC hDC;				/* device context */
    HGLRC hRC;				/* opengl context */
    HWND  hWnd;				/* window */
    MSG   msg;				/* message */

    char *windowName = (char *)"OpenGL Terrain Demo [2011 - psastras]";
    hWnd = CreateOpenGLWindow(windowName, 100, 100, properties.width, properties.height, PFD_TYPE_RGBA  , PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
			      PFD_DOUBLEBUFFER);
    if (hWnd == NULL) exit(1);

    hDC = GetDC(hWnd);
 
    
    int attributes[] =
        {
            WGL_SAMPLE_BUFFERS_ARB,  1,
            WGL_COLOR_SAMPLES_NV,    0,
            WGL_COVERAGE_SAMPLES_NV, 0,
            WGL_DOUBLE_BUFFER_ARB,   1,
            0, 0
        };
    int returnedPixelFormat = 0;
    UINT numFormats = 0;
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");	
    if(wglChoosePixelFormatARB) wglChoosePixelFormatARB(hDC, attributes,0 , 1, &returnedPixelFormat, &numFormats);
    hRC = wglCreateContext(hDC);//wglCreateContextAttribsARB(hDC, NULL, attributes); @todo: this causes a link error with my glew :S

    wglMakeCurrent(hDC, hRC);
    glewInit();
    HFONT font = CreateFont(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_TT_ONLY_PRECIS,
		      CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE|DEFAULT_PITCH,
		      "Calibri");
    (HFONT)SelectObject(hDC, font);
    GLint base = glGenLists(256);
    wglUseFontBitmaps(hDC, 0, 256, base);
    pEngine = new GLEngine(properties);
    ShowWindow(hWnd, 1);
    SetFocus(hWnd);
    float dt = 0.f;
    ShowCursor(false);

    const GLubyte *glVersionString = glGetString(GL_VERSION); // Get the version of OpenGL we are using
    int glVersion[2] = {-1, -1}; // Set some default values for the version
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); // Get back the OpenGL MAJOR version we are using
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]); // Get back the OpenGL MAJOR version we are using

    while (1) {
	while(PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE)) {
	    if(GetMessage(&msg, hWnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    } else {
		goto quit;
	    }
	}
	SYSTEMTIME st;
	GetSystemTime(&st);


	POINT pt;
	pt.x = pEngine->width() / 2;
	pt.y = pEngine->height() / 2;
	POINT lpt;
	GetCursorPos(&lpt);
	ClientToScreen(hWnd, &pt);
	SetCursorPos(pt.x, pt.y);
	pEngine->mouseMove(lpt.x - pt.x, pt.y - lpt.y, dt / 1000.f);

	StartCounter();
	pEngine->draw(st.wMinute * 60 + st.wSecond + st.wMilliseconds / 1000.f,
		      dt / 1000.f, pKeyController);
	dt = GetCounter();
	
	// text drawing
	{
	    std::stringstream ss;
	    ss << "OpenGL " << glVersion[0] << "." << glVersion[1];
	    const char *s = ss.str().c_str();
	    glPushAttrib(GL_LIST_BIT);
	    glRasterPos2f(10.f, 20.f);
	    glListBase(base);
	    glCallLists(ss.str().length(), GL_UNSIGNED_BYTE, s);
	    glListBase(0);
	    glPopAttrib();
	}
/*	
	{
	    std::stringstream ss;
	    if(dt < 0) dt = 0.01;
	    ss << (int)(dt) << " fps"; //@todo: this fps is totally broken its kind of funny
	    const char *s = ss.str().c_str();
	    glPushAttrib(GL_LIST_BIT);
	    glRasterPos2f(10.f, 40.f);
	    glListBase(base);
	    glCallLists(ss.str().length(), GL_UNSIGNED_BYTE, s);
	    glListBase(0);
	    glPopAttrib();
	}
	*/
	glFinish();
	glFlush();
	SwapBuffers(hDC);

	if(pKeyController->isKeyDown(27)) { //esc
	    break;
	}

	pKeyController->swapBuffers();
    }
quit:
    delete pEngine;
    glDeleteLists(base, 256);
    wglMakeCurrent(NULL, NULL);
    ReleaseDC(hWnd, hDC);
    wglDeleteContext(hRC);
    DestroyWindow(hWnd);

    return msg.wParam;
}
#else
bool checkGLXExtension(const char* extName, Display *dpy, int screen) {
  /*
    Search for extName in the extensions string.  Use of strstr()
    is not sufficient because extension names can be prefixes of
    other extension names.  Could use strtok() but the constant
    string returned by glGetString can be in read-only memory.
  */

  char* list = (char*) glXQueryExtensionsString(dpy, screen);
  char* end;
  int extNameLen;

  extNameLen = strlen(extName);
  end = list + strlen(list);

  while (list < end)  {
    int n = strcspn(list, " ");
    if ((extNameLen == n) && (strncmp(extName, list, n) == 0))
      return true;
    list += (n + 1);
  };
  return false;
};

int main(int argc, char *argv[]) {
    int which = PRIO_PROCESS;
    id_t pid;
    int priority = -20;
    int ret;

    pid = getpid();
    ret = setpriority(which, pid, priority);

     Display *dpy = XOpenDisplay(NULL);

     if(dpy == NULL) {
	    cerr << "Cannot connect to X server.  (Are you running a window system?)" << endl;
	    exit(1);
     }
     Window root = DefaultRootWindow(dpy);
     GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
     XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
     if(vi == NULL) {
	    cerr << "No appropriate visual information found (OpenGL context intialization failed).  Exiting." << endl;
	    exit(1);
     }

     Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
     XSetWindowAttributes swa;
     swa.colormap = cmap;
     swa.event_mask = ExposureMask | KeyPressMask;

     Window win = XCreateWindow(dpy, root, 0, 0, 1366, 768, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
     XMapWindow(dpy, win);
     XStoreName(dpy, win, "OpenGL Water Demo [2011 - psastras]");
     GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
     glXMakeCurrent(dpy, win, glc);
     XWindowAttributes gwa;
     XEvent xev;
     timespec ts;

     WindowProperties properties = {1366, 768};

     // load fonts
     XFontStruct *font = XLoadQueryFont(dpy, "fixed");
     GLuint listbase = glGenLists(96);
     glXUseXFont(font->fid, ' ', 96, listbase);

     // test for vsync
     void (*swapInterval)(int) = 0;
     if (checkGLXExtension("GLX_MESA_swap_control", dpy, 0))
       swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
     else if (checkGLXExtension("GLX_SGI_swap_control", dpy, 0))
       swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
     else
       cerr << "No vsync detected, disabling." << endl;
     if(swapInterval) swapInterval(1);
     float dt = 0.f;

     XSelectInput(dpy, win, KeyPressMask | KeyReleaseMask); //subscribe to keypress and release

     //hide cursor
     Cursor invisibleCursor;
     Pixmap bitmapNoData;
     XColor black;
     static char noData[] = { 0,0,0,0,0,0,0,0 };
     black.red = black.green = black.blue = 0;

     bitmapNoData = XCreateBitmapFromData(dpy, win, noData, 8, 8);
     invisibleCursor = XCreatePixmapCursor(dpy, bitmapNoData, bitmapNoData,
					  &black, &black, 0, 0);
     XDefineCursor(dpy,win, invisibleCursor);
     XFreeCursor(dpy, invisibleCursor);

    // main event loop
     while(1) {
	    if(XPending(dpy)) { //if we have a pending xevent
		XNextEvent(dpy, &xev);
		if(xev.type == Expose) { // gl context resized
			XGetWindowAttributes(dpy, win, &gwa);
			pEngine->resize(gwa.width, gwa.height);
			XWarpPointer(dpy, win, win, 0, 0, gwa.width, gwa.height, gwa.width / 2, gwa.height / 2);
		} else if(xev.type == KeyPress ) {
		    //cout << "press: " << xev.xkey.keycode << endl;
		    keycontroller->keyPressEvent(xev.xkey.keycode);
		} else if(xev.type == KeyRelease) {
		//    cout << "release: " << xev.xkey.keycode << endl;
		    keycontroller->keyReleaseEvent(xev.xkey.keycode);
		}
	    }


	    Window winfocused;
	    int    revert;
	    XGetInputFocus(dpy, &winfocused, &revert);
	    if(winfocused == win) { //if window is focused keep the mouse steady and respond to mouse events
		XQueryPointer(dpy, win, &xev.xbutton.root, &xev.xbutton.window, &xev.xbutton.x_root, &xev.xbutton.y_root, &xev.xbutton.x, &xev.xbutton.y, &xev.xbutton.state);
		XWarpPointer(dpy, win, win, 0, 0, gwa.width, gwa.height, gwa.width / 2, gwa.height / 2);
		float dx = (xev.xbutton.x - (gwa.width / 2.0)) / (float)gwa.width;
		float dy = (xev.xbutton.y - (gwa.height / 2.0)) / (float)gwa.height;
		pEngine->mouseMove(dx, dy, dt);
	    }

	    // handle keyboard - @todo: remove hard coded values
	    if(keycontroller->isKeyDown(9)) { //esc
		break;
	    }
	    //draw
	    clock_gettime(CLOCK_REALTIME, &ts);
	    long time = ts.tv_nsec;

	    pEngine->draw(ts.tv_sec, dt, keycontroller);

	    clock_gettime(CLOCK_REALTIME, &ts);
	    std::stringstream ss;
	    dt = ((ts.tv_nsec - time) * 1.0e-9);
	    if(dt < 0) dt = 0.01; //@todo this is cause were overflowing max long i think?
	    ss << (int)(1.0 / dt) << " fps";
	    const char *s = ss.str().c_str();
	    glPushAttrib(GL_LIST_BIT);
	    glListBase(listbase - 32);
	    glRasterPos2f(10.0, 20.0);
	    glCallLists(strlen(s), GL_BYTE, s);
	    glPopAttrib();
	    glListBase(0);

	    glXSwapBuffers(dpy, win);
	    glFinish();

	    keycontroller->swapBuffers();

    }

     delete keycontroller;
     // Restore the X left facing cursor
     Cursor cursor;
     cursor=XCreateFontCursor(dpy,XC_left_ptr);
     XDefineCursor(dpy, win, cursor);
     XFreeCursor(dpy, cursor);
     XUndefineCursor(dpy, win);
     glXMakeCurrent(dpy, None, NULL);
     glXDestroyContext(dpy, glc);
     XDestroyWindow(dpy, win);
     XCloseDisplay(dpy);
     return 0;
}
#endif
