#include <windows.h>
#include "blibrary.h"
#include "bloader.h"
#include <stdio.h>
#include "include\cef_base.h"

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"

#include "include/cef_base.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include <stdlib.h>
#include <thread>
#include "glapi.h"
#include <detours.h>
#include <gl\GL.h>
#include "glext.h"

#pragma comment(lib, "libcef.lib")
#pragma comment(lib, "libcef_dll_wrapper.lib")
#pragma comment(lib, "bloader.lib")

struct TextureObject
{
	TextureObject *next;
	TextureObject *prev;
	TextureObject *hashNext;
	unsigned int texGLName;
	unsigned int smallTexGLName;
	const char *texFileName;
	DWORD *type_GBitmap_bitmap;
	unsigned int texWidth;
	unsigned int texHeight;
	unsigned int bitmapWidth;
	unsigned int bitmapHeight;
	unsigned int downloadedWidth;
	unsigned int downloadedHeight;
	unsigned int enum_TextureHandleType_type;
	bool filterNearest;
	bool clamp;
	bool holding;
	int refCount;
};

static blmodule* us;

CefRefPtr<CefBrowser> brw;

std::thread blb; //The main loop thread

typedef int(*swapBuffersFn)();
static int texID = 0; //Texture ID that we bind to with OpenGL

static bool dirty = false; //Do we need to render the texture?

static int global_ww = 1024; //Width
static int global_hh = 768; //Height, both are used in swapBuffers to determine what should be copied over.

MologieDetours::Detour<swapBuffersFn>* swapBuffers_detour; //The detour so we can draw our stuff before Torque can.

GLuint* texBuffer;

int __fastcall swapBuffers_hook() {
	if (texID != 0 && dirty) {
		BL_glBindTexture(GL_TEXTURE_2D, texID);
		BL_glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, global_ww, global_hh, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer);
		BL_glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 
		/*
		* Yes, this will result in some issues with VERY old graphics cards.
		* But, if you're using a GPU that doesn't support generating mipmaps, should you really be using this?
		* Think about it. You're using the modern-day equivalent of Chromium, on a potato graphics card. It shouldn't be MY job to add in backwards compatibility for your potato.
		* It should be your responsibility to get a graphics card that has been released in the past few millenia.
		*/
		BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		dirty = false;
	}
	int ret = swapBuffers_detour->GetOriginalFunction()();
	return ret;
}

class BLBrowser : public CefApp {
public:
	BLBrowser() { };
	// CefBrowserProcessHandler methods:
	//rtual void OnRegisterCustomSchemes() OVERRIDE;
private:
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(BLBrowser);
};

class BLBrowserDrawer : public CefRenderHandler {
public:
	BLBrowserDrawer(int w, int h) :
		height(h), width(w)
	{
		if (!BL_glGenBuffers) {
			bloader_printf_error("Could not find genBuffers!");
			if (!BL_glGenBuffersARB) {
				bloader_printf_error("Could not find BL_genBuffersArb!");
				texBuffer = (GLuint*)malloc(2048 * 2048 * 4);
				memset((void*)texBuffer, 0, 2048 * 2048 * 4);
			}
			else {
				BL_glGenBuffersARB(1, &*texBuffer);
				BL_glBindBufferARB(GL_TEXTURE_BUFFER, *texBuffer);
			}
		}
		else {
		}
	};

	~BLBrowserDrawer() {
		if (BL_glDeleteBuffersARB) {
			BL_glDeleteBuffersARB(1, &*texBuffer);
		}
		else {
			delete texBuffer;
		}
	};

	CefRefPtr< CefAccessibilityHandler > GetAccessibilityHandler() {
		return nullptr;
	}

	bool GetRootScreenRect(CefRefPtr< CefBrowser > browser, CefRect& rect) {
		rect = CefRect(0, 0, width, height);
		return true;
	}


	bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
	{
		rect = CefRect(0, 0, width, height);
		return true;
	}

	bool GetScreenInfo(CefRefPtr< CefBrowser > browser, CefScreenInfo& screen_info) {
		screen_info.rect = CefRect(0, 0, width, height);
		screen_info.device_scale_factor = 1.0;
		return true;
	}

	void OnCursorChange(CefRefPtr< CefBrowser > browser, CefCursorHandle cursor, CefRenderHandler::CursorType type, const CefCursorInfo& custom_cursor_info) {

	}

	void OnImeCompositionRangeChanged(CefRefPtr< CefBrowser > browser, const CefRange& selected_range, const CefRenderHandler::RectList& character_bounds) {

	}

	void OnPaint(CefRefPtr< CefBrowser > browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int w, int h) {
		if (texID != 0) {
			if (BL_glBindBufferARB) {
				BL_glBindBufferARB(GL_TEXTURE_BUFFER, *texBuffer);
				BL_glBufferDataARB(GL_TEXTURE_BUFFER, width * height * 4, buffer, GL_DYNAMIC_DRAW);
			}
			else {
				memcpy(texBuffer, buffer, w * h * 4);
			}

			dirty = true;
			//texBuffer_0 = (char*)buffer;

			//sdirty = true;
			//bloader_printf("OnPaint");
		}
	}

	void OnPopupShow(CefRefPtr< CefBrowser > browser, bool show) {
		//nah, no popups for you.
	}

	void OnPopupSize(CefRefPtr< CefBrowser > browser, const CefRect& rect) {
		//nah, no popups.
	}

	void OnScrollOffsetChanged(CefRefPtr< CefBrowser > browser, double x, double y) {

	}

	bool StartDragging(CefRefPtr< CefBrowser > browser, CefRefPtr< CefDragData > drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y) {
		return false;
	}

	void UpdateDragCursor(CefRefPtr< CefBrowser > browser, CefRenderHandler::DragOperation operation) {

	}

	void UpdateResolution(int hh, int ww) {

		if (hh * ww * 4 > 16777215) {
			bloader_printf_error("That's too damn big.");
			return;
		}
		
		memset(texBuffer, 0, 2048 * 2048 * 4);
		height = hh;
		width = ww;
		global_ww = width;
		global_hh = height;

	}

private:
	IMPLEMENT_REFCOUNTING(BLBrowserDrawer);
	int height, width;
};

bool* run = new bool(true);

void id(blinfo* info) {
	const char* description = "Author: hatf0/Metario/w[hat]#0518";
	const char* name = "BLBrowser^2";

	strncpy_s(info->description, description, strlen(description));
	strncpy_s(info->name, name, strlen(name));
	info->version = 1;
}

bool bindTexID() {
	TextureObject* texture;
	const char* string = "Add-Ons/Print_Screen_Cinema/prints/Cinema.png";
	texID = 0;
	for (texture = (TextureObject*)0x7868E0; texture; texture = texture->next) {
		if (texture->texFileName != NULL && _stricmp(texture->texFileName, string) == 0) {
			texID = texture->texGLName;
			bloader_printf("Found textureID; %u", texture->texGLName);
			return true;
		}
	}
	return false;
}

class BrowserClient :
	public CefClient,
	public CefLifeSpanHandler,
	public CefLoadHandler
{
public:
	BrowserClient(CefRefPtr<CefRenderHandler> ptr) :
		handler(ptr)
	{
	}

	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()
	{
		return this;
	}

	virtual CefRefPtr<CefLoadHandler> GetLoadHandler()
	{
		return this;
	}

	virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
	{
		return handler;
	}

	// CefLifeSpanHandler methods.
	void OnAfterCreated(CefRefPtr<CefBrowser> browser)
	{
		// Must be executed on the UI thread.
		//CEF_REQUIRE_UI_THREAD();

		browser_id = browser->GetIdentifier();
	}

	bool DoClose(CefRefPtr<CefBrowser> browser)
	{
		// Must be executed on the UI thread.
		//CEF_REQUIRE_UI_THREAD();

		// Closing the main window requires special handling. See the DoClose()
		// documentation in the CEF header for a detailed description of this
		// process.
		if (browser->GetIdentifier() == browser_id)
		{
			// Set a flag to indicate that the window close should be allowed.
			closing = true;
		}

		// Allow the close. For windowed browsers this will result in the OS close
		// event being sent.
		return false;
	}

	void OnBeforeClose(CefRefPtr<CefBrowser> browser)
	{
	}

	void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
	{
		//std::cout << "OnLoadEnd(" << httpStatusCode << ")" << std::endl;
		loaded = true;
	}

	bool OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString & failedUrl, CefString & errorText)
	{
		//std::cout << "OnLoadError()" << std::endl;
		loaded = true;
	}

	void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
	{
		//std::cout << "OnLoadingStateChange()" << std::endl;
	}

	void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
	{
		//std::cout << "OnLoadStart()" << std::endl;
	}

	bool closeAllowed() const
	{
		return closing;
	}

	bool isLoaded() const
	{
		return loaded;
	}

private:
	int browser_id;
	bool closing = false;
	bool loaded = false;
	CefRefPtr<CefRenderHandler> handler;

	IMPLEMENT_REFCOUNTING(BrowserClient);
};


/*
* are you happy now, clay hanson?
*/

bool bindToTexture(void* this_, int argc, const char* argv[]) {
	return bindTexID();
}

void visitURL(void* this_, int argc, const char* argv[]) {
	if (brw.get() != nullptr) {
		brw->GetMainFrame()->LoadURL(CefString(argv[1]));
	}
	else {
		bloader_printf_error("brw was a nullptr");
	}
}

void executeJS(void* this_, int argc, const char* argv[]) {
	if (brw.get() != nullptr) {
		brw->GetMainFrame()->ExecuteJavaScript(CefString(argv[1]), CefString(""), 1);
	}
	else {
		bloader_printf_error("brw was a nullptr");
	}
}


CefRefPtr<BLBrowserDrawer> renderHandler;

void resizeWindow(void* this_, int argc, const char* argv[]) {
	int width = atoi(argv[1]);
	int height = atoi(argv[2]);

	renderHandler->UpdateResolution(width, height);

	brw->GetHost()->WasResized();
}

void mouseMove(void* this_, int argc, const char* argv[]) {
	CefMouseEvent* evt = new CefMouseEvent();
	evt->x = atoi(argv[1]);
	evt->y = atoi(argv[2]);
	brw->GetHost()->SendMouseMoveEvent(*evt, false);

	delete evt;
}

void mouseClick(void* this_, int argc, const char* argv[]) {
	CefMouseEvent* evt = new CefMouseEvent();
	evt->x = atoi(argv[1]);
	evt->y = atoi(argv[2]);
	
	int clickType = atoi(argv[3]);
	brw->GetHost()->SendMouseClickEvent(*evt, (cef_mouse_button_type_t)clickType, false, 1);
	brw->GetHost()->SendMouseClickEvent(*evt, (cef_mouse_button_type_t)clickType, true, 1);

	delete evt;
}

void mouseWheel(void* this_, int argc, const char* argv[]) {
	CefMouseEvent* evt = new CefMouseEvent();
	evt->x = atoi(argv[1]);
	evt->y = atoi(argv[2]);

	int deltaX = atoi(argv[3]);
	int deltaY = atoi(argv[4]);
	brw->GetHost()->SendMouseWheelEvent(*evt, deltaX, deltaY);

	delete evt;
}

void keyboardEvent(void* this_, int argc, const char* argv[]) {
	CefKeyEvent* evt = new CefKeyEvent();
	evt->character = argv[1][0];
	evt->modifiers = atoi(argv[2]);
	brw->GetHost()->SendKeyEvent(*evt);

	delete evt;
}

void runml(bool* dowecontinue) { //Run the main loop here.
	CefMainArgs args;

	CefSettings settings;
	settings.single_process = false;
	settings.command_line_args_disabled = true;
	settings.no_sandbox = true;
	CefString(&settings.browser_subprocess_path).FromASCII("blbrowser_subproc.exe");
	CefString(&settings.user_agent).FromASCII("Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.146 Safari/537.36 Blockland/r1986 (Torque Game Engine/1.3)");
	settings.windowless_rendering_enabled = true;
	if (!CefInitialize(args, settings, new BLBrowser(), nullptr))
		bloader_printf_error("Failed to init CEF.");
	else {
		renderHandler = new BLBrowserDrawer(global_ww, global_hh);
		CefBrowserSettings browser_settings;
		CefWindowInfo window_info;
		CefRefPtr<BrowserClient> browserClient;
		browserClient = new BrowserClient(renderHandler);
		browser_settings.windowless_frame_rate = 30;
		window_info.SetAsWindowless(0);
		brw = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), "https://google.com", browser_settings, NULL);
		while (true) {
			if (*dowecontinue == false) {
				brw->GetHost()->CloseBrowser(true);
				CefShutdown();
				bloader_printf("Unloaded");
				return;
			}
			CefDoMessageLoopWork();
			Sleep(1);
			//bloader_printf("Alive");
		}
		//CefQuitMessageLoop();
	}
}

extern "C" {
	int blibrary_initialize(blmodule* module) {
		us = module;
		bloader_consolefunction_bool(us, "", "CEF_bindTex", bindToTexture, "() - Bind to the texture representing a Cinema screen.", 1, 1);
		bloader_consolefunction_void(us, "", "CEF_goToURL", visitURL, "(string url) - Visit a URL", 2, 2);
		bloader_consolefunction_void(us, "", "clientCmdCEF_goToURL", visitURL, "(string url) - Visit a URL", 2, 2);
		bloader_consolefunction_void(us, "", "CEF_executeJS", executeJS, "(string code) - Execute JavaScript on the current window.", 2, 2);
		bloader_consolefunction_void(us, "", "CEF_resizeWindow", resizeWindow, "(int width, int height) - Resize the CEF window, reallocating the texture buffer.", 3, 3);
		bloader_consolefunction_void(us, "", "CEF_mouseMove", mouseMove, "(int x, int y) - Move the mouse to this position.", 3, 3);

		bloader_consolefunction_void(us, "", "CEF_mouseClick", mouseClick, "(int x, int y, int clickType) - Send a click event on the specified coordinates.", 4, 4);
		bloader_consolefunction_void(us, "", "CEF_mouseWheel", mouseWheel, "(int x, int y, int deltaX, int deltaY) - Send a mousewheel event at the coords.", 5, 5);
		bloader_consolefunction_void(us, "", "CEF_keyboardEvent", keyboardEvent, "(char key, int modifiers) - Send a keyboard event to CEF.", 3, 3);

		initGL();
		swapBuffers_detour = new MologieDetours::Detour<swapBuffersFn>((swapBuffersFn)0x4237D0, (swapBuffersFn)swapBuffers_hook);
		blb = std::thread(runml, run);

		bloader_printf("BLBrowser^2 ready for action!");
		return BL_OK;
	}

	void blibrary_info(blinfo* info) {
		id(info);
	}
	
	void blibrary_deinit() {
		bloader_printf("We should detach here");
		if (*run) {
			*run = false;
			blb.join();
		}
		delete run;
		free(texBuffer);
	}

	int WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
	{
		if (reason == DLL_PROCESS_ATTACH)
			return true;
		else if (reason == DLL_PROCESS_DETACH)
			blibrary_deinit();
			return true;

		return true;
	}
}
