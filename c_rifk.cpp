#include "c_rifk.h"
#include "hooks/c_hooks.h"
#include "hooks/idirect3ddevice9.h"
#include "menu/c_menu.h"
#include <thread>

using namespace std::chrono_literals;

__declspec(dllexport) rifk_loader_info loader_info {};

c_rifk::c_rifk()
{
	c_menu::instance();
	c_netvar::instance();
	c_hooks::run();
}

void runshit(void) {
	c_rifk::instance();
}

DWORD WINAPI OnDllAttach(LPVOID base)
{
	while (!GetModuleHandleA("serverbrowser.dll"))
		Sleep(75);

	CreateDirectory("C:/rifk7/", NULL);
	CreateDirectory("C:/rifk7/cfg/", NULL);
	try {
		playback_loop();
		FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
	}
	catch (const std::exception & ex) {
		FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
	}
}

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDll,
	_In_ DWORD fdwReason,
	_In_opt_ LPVOID lpvReserved
)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDll);
		CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);
		return TRUE;
	default:
		return TRUE;
	}
}
