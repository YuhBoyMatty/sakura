#include "client.h"

glBegin_t pglBegin = NULL;
wglSwapBuffers_t pwglSwapBuffers = NULL;
glClear_t pglClear = NULL;
glColor4f_t pglColor4f = NULL;
glReadPixels_t pglReadPixels = NULL;
glVertex3fv_t pglVertex3fv = NULL;

bool bSmoke = false;

void APIENTRY Hooked_glBegin(GLenum mode)
{
	if (cvar.visual_nosmoke)
	{
		if (mode == GL_QUADS)
		{
				GLfloat smokecol[4];

				glGetFloatv(GL_CURRENT_COLOR, smokecol);

				if ((smokecol[0] == smokecol[1]) && (smokecol[0] == smokecol[2]) && (smokecol[0] != 0.0) && smokecol[0] != 1.0)
					bSmoke = true;
				else
					bSmoke = false;
		}
	}

	pglBegin(mode);
}

void APIENTRY Hooked_glVertex3fv(GLfloat* v)
{
	if(bSmoke)
		return;

	pglVertex3fv(v);
}

BOOL APIENTRY Hooked_wglSwapBuffers(HDC hdc)
{
	if (hdc)
		HookImGui(hdc);

	return pwglSwapBuffers(hdc);
}

void APIENTRY Hooked_glClear(GLbitfield mask)
{
	if (mask == GL_DEPTH_BUFFER_BIT)
	{
		mask |= GL_DEPTH_BUFFER_BIT;
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	pglClear(mask);
}

void APIENTRY Hooked_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
	/*if (ScreenFirst || !cvar.snapshot_memory)
	{
		dwSize = (width * height) * 3;
		BufferScreen = (PBYTE)malloc(dwSize);
		pglReadPixels(x, y, width, height, format, type, pixels);
		memcpy(BufferScreen, pixels, dwSize);
		DrawVisuals = true;
		ScreenFirst = false;
	}
	memcpy(pixels, BufferScreen, dwSize);*/

	GLuint uiImageSize = width * height * 3;
	if (Sakura::ScreenShot::BufferScreen.size() && Sakura::ScreenShot::BufferScreen.size() <= uiImageSize)
		memcpy(pixels, Sakura::ScreenShot::BufferScreen.data(), uiImageSize);
	else
		memset(pixels, 0, uiImageSize);
}

void APIENTRY Hooked_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	if (Sakura::Chams::world)
	{
		if (cvar.chams_world == 1 || cvar.chams_world == 3)
			red = Sakura::Chams::worldColor.r, green = Sakura::Chams::worldColor.g, blue = Sakura::Chams::worldColor.b;
		if (cvar.chams_world == 2)
			red = Sakura::Chams::worldColor.r * red, green = Sakura::Chams::worldColor.g * green, blue = Sakura::Chams::worldColor.b * blue;
	}
	
	if (Sakura::Chams::viewmodel)
	{
		if (cvar.chams_view_model == 1 || cvar.chams_view_model == 3)
			red = Sakura::Chams::viewmodelColor.r, green = Sakura::Chams::viewmodelColor.g, blue = Sakura::Chams::viewmodelColor.b;
		if (cvar.chams_view_model == 2)
			red = Sakura::Chams::viewmodelColor.r * red, green = Sakura::Chams::viewmodelColor.g * green, blue = Sakura::Chams::viewmodelColor.b * blue;
	}

	if (Sakura::Chams::player)
	{
		if (cvar.chams_player == 1 || cvar.chams_player == 3)
			red = Sakura::Chams::playerColor.r, green = Sakura::Chams::playerColor.g, blue = Sakura::Chams::playerColor.b;
		if (cvar.chams_player == 2)
			red = Sakura::Chams::playerColor.r * red, green = Sakura::Chams::playerColor.g * green, blue = Sakura::Chams::playerColor.b * blue;
	}
	
	if (Sakura::Chams::playerFake)
	{
		if (cvar.misc_backtrack_chams == 1 || cvar.misc_backtrack_chams == 3)
			red = Sakura::Chams::playerFakeColor.r, green = Sakura::Chams::playerFakeColor.g, blue = Sakura::Chams::playerFakeColor.b;
		if (cvar.misc_backtrack_chams == 2)
			red = Sakura::Chams::playerFakeColor.r * red, green = Sakura::Chams::playerFakeColor.g * green, blue = Sakura::Chams::playerFakeColor.b * blue;
	}

	if (Sakura::Chams::localFakePlayer)
	{
		if (cvar.visual_fakelag_history_local_chams == 1 || cvar.visual_fakelag_history_local_chams == 3)
			red = Sakura::Chams::localFakePlayerColor.r, green = Sakura::Chams::localFakePlayerColor.g, blue = Sakura::Chams::localFakePlayerColor.b;
		if (cvar.visual_fakelag_history_local_chams == 2)
			red = Sakura::Chams::localFakePlayerColor.r * red, green = Sakura::Chams::localFakePlayerColor.g * green, blue = Sakura::Chams::localFakePlayerColor.b * blue;
	}

	if (Sakura::Chams::localPlayer)
	{
		if (cvar.chams_local == 1 || cvar.chams_local == 3)
			red = Sakura::Chams::localPlayerColor.r, green = Sakura::Chams::localPlayerColor.g, blue = Sakura::Chams::localPlayerColor.b;
		if (cvar.chams_local == 2)
			red = Sakura::Chams::localPlayerColor.r * red, green = Sakura::Chams::localPlayerColor.g * green, blue = Sakura::Chams::localPlayerColor.b * blue;
	}

	pglColor4f(red, green, blue, alpha);
}

void HookOpenGL()
{
	if (g_Studio.IsHardware() != 1)
		c_Offset.Error(/*Please run game in OpenGL renderer mode*/XorStr<0xC7, 40, 0xBEF8CBD8>("\x97\xA4\xAC\xAB\xB8\xA9\xED\xBC\xBA\xBE\xF1\xB5\xB2\xB9\xB0\xF6\xBE\xB6\xF9\x95\xAB\xB9\xB3\x99\x93\xC0\x93\x87\x8D\x80\x80\x94\x82\x9A\xC9\x87\x84\x88\x88" + 0xBEF8CBD8).s);

	HMODULE hmOpenGL = GetModuleHandle("opengl32.dll"); 
	if (hmOpenGL)
	{
		// no smoke works.
		// well its only for my intel hd graphics, i can guess its pointless to compile it with it and share
		// -- thanks for bloodsharp
		//pglBegin = (glBegin_t)DetourFunction((LPBYTE)(DWORD)GetModuleHandle("ig7icd32.dll") + 0x4306C0, (LPBYTE)&Hooked_glBegin);
		//pglVertex3fv = (glVertex3fv_t)DetourFunction((PBYTE)(DWORD)GetModuleHandle("ig7icd32.dll") + 0x441540, (PBYTE)&Hooked_glVertex3fv);
		//pglClear = (glClear_t)DetourFunction((PBYTE)(DWORD)GetModuleHandle("ig7icd32.dll") + 0x431580, (LPBYTE)&Hooked_glClear);

		pglBegin = (glBegin_t)DetourFunction((LPBYTE)GetProcAddress(hmOpenGL, "glBegin"), (LPBYTE)&Hooked_glBegin);
		pglVertex3fv = (glVertex3fv_t)DetourFunction((PBYTE)GetProcAddress(hmOpenGL, "glVertex3fv"), (PBYTE)&Hooked_glVertex3fv);
		pwglSwapBuffers = (wglSwapBuffers_t)DetourFunction((LPBYTE)GetProcAddress(hmOpenGL, "wglSwapBuffers"), (LPBYTE)&Hooked_wglSwapBuffers);
		pglClear = (glClear_t)DetourFunction((LPBYTE)GetProcAddress(hmOpenGL, "glClear"), (LPBYTE)&Hooked_glClear);
		pglColor4f = (glColor4f_t)DetourFunction((LPBYTE)GetProcAddress(hmOpenGL, "glColor4f"), (LPBYTE)&Hooked_glColor4f);
		pglReadPixels = (glReadPixels_t)DetourFunction((PBYTE)GetProcAddress(hmOpenGL, "glReadPixels"), (PBYTE)Hooked_glReadPixels);
	}
}