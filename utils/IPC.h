#pragma once

#include <windows.h>
#include <string>

typedef struct filemap_s
{
	DWORD real_dll_size;
	char szDll[4096 * 2048];
	int write_counter;
	bool can_close_host;
	bool is_vmt_scan;
} filemap_t, * pFilemap_t;

filemap_t g_loader_ipc;

HANDLE g_hFilemap = NULL;
PVOID g_pFileView = NULL;

inline PVOID CreateMap(const std::string& MapName)
{
	if (g_pFileView)
		return g_pFileView;

	g_hFilemap = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_EXECUTE_READWRITE, 0, sizeof(filemap_t), MapName.c_str());
	if (!g_hFilemap || g_hFilemap == INVALID_HANDLE_VALUE)
		return 0;
	g_pFileView = MapViewOfFile(g_hFilemap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (!g_pFileView)
		return 0;
	return g_pFileView;
}

inline void ReadMap()
{
	int old_write_counter = g_loader_ipc.write_counter;

	if (((pFilemap_t)g_pFileView)->write_counter > old_write_counter)
	{
		memcpy(&g_loader_ipc, g_pFileView, sizeof(filemap_t));
	}
}

inline void WriteMap()
{
	g_loader_ipc.write_counter++;
	memcpy(g_pFileView, &g_loader_ipc, sizeof(filemap_t));
}

inline void ReleaseMap()
{
	if (g_pFileView)
		UnmapViewOfFile(g_pFileView);

	if (g_hFilemap)
		CloseHandle(g_hFilemap);
}