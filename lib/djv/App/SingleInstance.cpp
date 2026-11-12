// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/App/SingleInstance.h>

#include <djv/Models/Review.h>

#include <ftk/Core/Path.h>
#include <ftk/Core/String.h>

#if defined(_WIN32) && defined(FTK_SDL3)
#include <SDL3/SDL.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _WIN32

#include <cstring>
#include <filesystem>
#include <map>

namespace djv
{
    namespace app
    {
#if defined(_WIN32) && defined(FTK_SDL3)
        namespace
        {
            const wchar_t* className = L"DJV_Benn.Instance";

            //! Held by a run from the moment it decides to be the instance
            //! until it can take files, so that runs started together --
            //! several files opened at once from Explorer, a process each --
            //! wait for it rather than each becoming an instance.
            const wchar_t* startMutexName = L"Local\\DJV_Benn.InstanceStart";
            HANDLE startMutex = nullptr;

            //! Marks the data as DJV's file list: "DJV1".
            const ULONG_PTR copyDataID = 0x444A5631;

            //! The receiver's list, written by its window on the main thread.
            std::vector<std::string>* receivedPaths = nullptr;

            void releaseStartMutex()
            {
                if (startMutex)
                {
                    ReleaseMutex(startMutex);
                    CloseHandle(startMutex);
                    startMutex = nullptr;
                }
            }

            LRESULT CALLBACK receiverProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                if (WM_COPYDATA == msg)
                {
                    const auto cds = reinterpret_cast<const COPYDATASTRUCT*>(lParam);
                    if (cds && copyDataID == cds->dwData && receivedPaths)
                    {
                        // UTF-8 paths, each ended by a zero.
                        const char* data = static_cast<const char*>(cds->lpData);
                        size_t i = 0;
                        while (i < cds->cbData)
                        {
                            const size_t size = strnlen(data + i, cds->cbData - i);
                            if (size > 0)
                            {
                                receivedPaths->push_back(std::string(data + i, size));
                            }
                            i += size + 1;
                        }
                        return TRUE;
                    }
                }
                return DefWindowProcW(hwnd, msg, wParam, lParam);
            }

            //! The receiver of the instance used last: of the instances'
            //! windows, the one nearest the top.
            HWND findReceiver()
            {
                std::map<DWORD, HWND> receivers;
                HWND hwnd = nullptr;
                while ((hwnd = FindWindowExW(HWND_MESSAGE, hwnd, className, nullptr)))
                {
                    DWORD pid = 0;
                    GetWindowThreadProcessId(hwnd, &pid);
                    if (pid != GetCurrentProcessId())
                    {
                        receivers[pid] = hwnd;
                    }
                }
                if (receivers.empty())
                {
                    return nullptr;
                }
                struct Find
                {
                    const std::map<DWORD, HWND>* receivers = nullptr;
                    HWND out = nullptr;
                };
                Find find;
                find.receivers = &receivers;
                // Top-level windows come from the top down.
                EnumWindows(
                    [](HWND hwnd, LPARAM lParam) -> BOOL
                    {
                        auto find = reinterpret_cast<Find*>(lParam);
                        if (IsWindowVisible(hwnd))
                        {
                            DWORD pid = 0;
                            GetWindowThreadProcessId(hwnd, &pid);
                            const auto i = find->receivers->find(pid);
                            if (i != find->receivers->end())
                            {
                                find->out = i->second;
                                return FALSE;
                            }
                        }
                        return TRUE;
                    },
                    reinterpret_cast<LPARAM>(&find));
                return find.out ? find.out : receivers.begin()->second;
            }
        }
#endif // _WIN32

        bool forwardToRunningInstance(const std::vector<std::string>& argv)
        {
#if defined(_WIN32) && defined(FTK_SDL3)
            if (argv.size() < 2)
            {
                return false;
            }
            std::string data;
            for (size_t i = 1; i < argv.size(); ++i)
            {
                // An option makes the run one of its own: a script, a test,
                // "-newInstance".
                const std::string& arg = argv[i];
                if (arg.empty() || '-' == arg[0])
                {
                    return false;
                }
                // Made whole here: the other instance has a directory of its
                // own.
                std::error_code ec;
                const std::filesystem::path path =
                    std::filesystem::absolute(ftk::toFileSystem(arg), ec);
                if (ec)
                {
                    return false;
                }
                // A review replaces the whole session, which would be the
                // other instance's.
                if (path.extension() == models::reviewExtension())
                {
                    return false;
                }
                data += ftk::fromFileSystem(path);
                data.push_back(0);
            }

            startMutex = CreateMutexW(nullptr, FALSE, startMutexName);
            if (startMutex)
            {
                // Long enough for the slowest start; a run that never gets
                // there ends, and its mutex with it.
                const DWORD wait = WaitForSingleObject(startMutex, 30000);
                if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED)
                {
                    CloseHandle(startMutex);
                    startMutex = nullptr;
                }
            }
            if (HWND receiver = findReceiver())
            {
                DWORD pid = 0;
                GetWindowThreadProcessId(receiver, &pid);
                // Started by the user, this run may bring a window to the
                // front, and it passes that on.
                AllowSetForegroundWindow(pid);
                COPYDATASTRUCT cds;
                cds.dwData = copyDataID;
                cds.cbData = static_cast<DWORD>(data.size());
                cds.lpData = data.data();
                DWORD_PTR result = 0;
                if (SendMessageTimeoutW(
                    receiver,
                    WM_COPYDATA,
                    0,
                    reinterpret_cast<LPARAM>(&cds),
                    SMTO_ABORTIFHUNG,
                    10000,
                    &result) && result)
                {
                    releaseStartMutex();
                    return true;
                }
            }
            // This run is the instance; the mutex is held until its receiver
            // is made.
#endif // _WIN32
            return false;
        }

        struct InstanceReceiver::Private
        {
            std::vector<std::string> paths;
#if defined(_WIN32) && defined(FTK_SDL3)
            HWND hwnd = nullptr;
#endif // _WIN32
        };

        InstanceReceiver::InstanceReceiver() :
            _p(new Private)
        {
#if defined(_WIN32) && defined(FTK_SDL3)
            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(wc);
            wc.lpfnWndProc = receiverProc;
            wc.hInstance = GetModuleHandleW(nullptr);
            wc.lpszClassName = className;
            RegisterClassExW(&wc);
            // A message-only window: never shown, and found by its class.
            _p->hwnd = CreateWindowExW(
                0, className, L"", 0, 0, 0, 0, 0,
                HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
            receivedPaths = &_p->paths;
            releaseStartMutex();
#endif // _WIN32
        }

        InstanceReceiver::~InstanceReceiver()
        {
#if defined(_WIN32) && defined(FTK_SDL3)
            if (_p->hwnd)
            {
                DestroyWindow(_p->hwnd);
            }
            receivedPaths = nullptr;
#endif // _WIN32
        }

        std::vector<std::string> InstanceReceiver::takePaths()
        {
            std::vector<std::string> out;
            out.swap(_p->paths);
            return out;
        }

        void InstanceReceiver::raise()
        {
#if defined(_WIN32) && defined(FTK_SDL3)
            // The main window is the first made.
            int count = 0;
            if (SDL_Window** windows = SDL_GetWindows(&count))
            {
                if (count > 0)
                {
                    const SDL_PropertiesID props = SDL_GetWindowProperties(windows[0]);
                    if (HWND hwnd = static_cast<HWND>(SDL_GetPointerProperty(
                        props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)))
                    {
                        if (IsIconic(hwnd))
                        {
                            ShowWindow(hwnd, SW_RESTORE);
                        }
                        SetForegroundWindow(hwnd);
                    }
                }
                SDL_free(windows);
            }
#endif // _WIN32
        }
    }
}
