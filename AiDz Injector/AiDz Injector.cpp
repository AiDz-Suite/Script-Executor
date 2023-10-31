#include <Windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <filesystem>


void inject(const std::string& dll_name)
{
	TCHAR buffer[MAX_PATH]{ 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);

	std::string file_path{ buffer };
	file_path = file_path.substr(0, file_path.find_last_of('\\'));
	file_path += "\\" + dll_name;

	if (!std::filesystem::exists(file_path))
	{
		std::printf("Failed to find DLL!\n");
		return;
	}

	DWORD pid = 0;
	GetWindowThreadProcessId(FindWindowA("UnityWndClass", "BrickPlanet Client"), &pid);

	if (!pid)
	{
		std::printf("Failed to find BrickPlanet!\n");
		return;
	}
	
	std::printf("Found BrickPlanet: %d\n", pid);

	HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (h_process == INVALID_HANDLE_VALUE)
	{
		std::printf("Failed to open process!\n");
		return;
	}

	void* injected_dll_path = VirtualAllocEx(h_process, nullptr, file_path.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!injected_dll_path)
	{
		std::printf("Failed to write memory!\n");
		return;
	}

	WriteProcessMemory(h_process, injected_dll_path, file_path.c_str(), file_path.size(), nullptr);

	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	if (h_snapshot == INVALID_HANDLE_VALUE)
	{
		std::printf("Failed to get process information!\n");
		return;
	}

	THREADENTRY32 current_thread{};
	current_thread.dwSize = sizeof(current_thread);

	if (Thread32First(h_snapshot, &current_thread))
	{
		do
		{
			if (current_thread.th32OwnerProcessID != pid)
				continue;

			HANDLE h_thread = OpenThread(THREAD_ALL_ACCESS, TRUE, current_thread.th32ThreadID);
			if (h_thread == INVALID_HANDLE_VALUE)
				continue;

			if (QueueUserAPC(reinterpret_cast<PAPCFUNC>(&LoadLibraryA), h_thread, reinterpret_cast<ULONG_PTR>(injected_dll_path)))
			{
				std::printf("APC queued for thread %d\n", current_thread.th32ThreadID);
				CloseHandle(h_thread);
				break;
			}

			CloseHandle(h_thread);
		} while (Thread32Next(h_snapshot, &current_thread));
	}
	CloseHandle(h_snapshot);
	CloseHandle(h_process);
}

int main()
{
	SetConsoleTitleA("AiDz Injector");
	std::printf("Stealthily injecting REAL AIDS into your game instance.\n");

	inject("aids.dll");
}