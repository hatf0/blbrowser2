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

std::thread blb;

typedef int(*swapBuffersFn)();
static int texID = 0;

static bool dirty = false;

static int ww = 1024;
static int hh = 768;

MologieDetours::Detour<swapBuffersFn>* swapBuffers_detour;

GLuint* texBuffer;

int __fastcall swapBuffers_hook() {

	if (texID != 0 && dirty) {
		BL_glBindTexture(GL_TEXTURE_2D, texID);
		BL_glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ww, hh, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer);
		/*if (!BL_glGenerateMipmap) {
			BL_glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 512, 512, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_1);
			BL_glTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, 256, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_2);
			BL_glTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, 128, 128, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_3);
			BL_glTexSubImage2D(GL_TEXTURE_2D, 4, 0, 0, 64, 64, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_4);
			BL_glTexSubImage2D(GL_TEXTURE_2D, 5, 0, 0, 32, 32, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_5);
			BL_glTexSubImage2D(GL_TEXTURE_2D, 6, 0, 0, 16, 16, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_6);
		}
		else {
			BL_glGenerateMipmap(GL_TEXTURE_2D);*/
			BL_glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
			BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//}
		dirty = false;
	}
		int ret = swapBuffers_detour->GetOriginalFunction()();
	return ret;
}

//Stolen blatantly from BLBrowser lol
//char *texBuffer_1 = new char[512 * 512 * 4];
//char *texBuffer_2 = new char[256 * 256 * 4];
//char *texBuffer_3 = new char[128 * 128 * 4];
//char *texBuffer_4 = new char[64 * 64 * 4];
//char *texBuffer_5 = new char[32 * 32 * 4];
//char *texBuffer_6 = new char[16 * 16 * 4];


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
				texBuffer = (GLuint*)malloc(width * height * 4);
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
				memcpy(texBuffer, buffer, width * height * 4);
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

private:
	IMPLEMENT_REFCOUNTING(BLBrowserDrawer);
	int height, width;
};

bool* run = new bool(true);

void id(blinfo* info) {
	info->description = (char*)"fuck";
	info->name = (char*)"BLBrowser";
	info->version = 1;
}

bool bindTexID() {
	TextureObject* texture;
	const char* string = "Add-Ons/Print_Screen_Cinema/prints/Fuck.png";
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


bool fuckass(void* this_, int argc, const char* argv[]) { //todo, make this function name better
	if (brw.get() != nullptr) {
		brw->GetMainFrame()->LoadURL(CefString(argv[1]));
	}
	else {
		bloader_printf("brw was null");
	}

	if (!BL_glGenerateMipmap) {
		bloader_printf("unoptimized mipmap grabbing");
	}
	return bindTexID();
}

void runml(bool* dowecontinue) {
	CefMainArgs args;

	CefSettings settings;
	settings.single_process = false;
	settings.command_line_args_disabled = true;
	settings.no_sandbox = true;
	CefString(&settings.browser_subprocess_path).FromASCII("blbrowser_subproc.exe");
	CefString(&settings.user_agent).FromASCII("Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.103 Safari/537.36 GMod/13 (CEF, Blockland)");
	settings.windowless_rendering_enabled = true;
	if (!CefInitialize(args, settings, new BLBrowser(), nullptr))
		bloader_printf_error("Failed to init CEF.");
	else {
		CefRefPtr<BLBrowserDrawer> renderHandler = new BLBrowserDrawer(ww, hh);
		CefBrowserSettings browser_settings;
		CefWindowInfo window_info;
		CefRefPtr<BrowserClient> browserClient;
		browserClient = new BrowserClient(renderHandler);
		browser_settings.windowless_frame_rate = 60;
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
		bloader_printf("BLBrowser ready for action!");
		bloader_consolefunction_bool(us, "", "bindTex", fuckass, "()", 2, 2);
		initGL();
		swapBuffers_detour = new MologieDetours::Detour<swapBuffersFn>((swapBuffersFn)0x4237D0, (swapBuffersFn)swapBuffers_hook);
		blb = std::thread(runml, run);
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
