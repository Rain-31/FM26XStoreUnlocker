#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <tlhelp32.h>
#include "logger.h"

// ---------------------------------------------------------
// VEH Hardware Hook Implementation
// ---------------------------------------------------------

static uintptr_t g_targetFuncAddr = 0;

LONG WINAPI VehHandler(PEXCEPTION_POINTERS ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
        if (g_targetFuncAddr != 0 && ExceptionInfo->ContextRecord->Rip == g_targetFuncAddr) {
            
            void* _this = (void*)ExceptionInfo->ContextRecord->Rcx;
            void* evt = (void*)ExceptionInfo->ContextRecord->Rdx;
            
            LOG_INFO("[VEH Hook] Hardware Breakpoint HIT! _this=%p, evt=%p", _this, evt);
            
            if (evt) {
                // Read the event ID based on dump.cs: private uint m_id; // 0x18
                uint32_t eventId = *(uint32_t*)((uintptr_t)evt + 0x18);
                LOG_INFO("[VEH Hook] Event ID intercepted: %u", eventId);

                // If eventId is 0, it might be an initialization struct or we need to dump it
                if (eventId == 0) {
                    LOG_INFO("[VEH Hook] Got ID 0, dumping some bytes around evt to find real data");
                    uint32_t* p = (uint32_t*)evt;
                    LOG_INFO("[VEH Hook] Dump: %08X %08X %08X %08X %08X %08X %08X %08X", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
                }

                if (eventId == 83) { // EventFmxInGameStorePurchaseCompleted
                    LOG_INFO("[VEH Hook] Forcing Event 83 verification result to SUCCESS");
                    // Modify TypedValue <Value>k__BackingField; // 0x20
                    *(uint64_t*)((uintptr_t)evt + 0x20) = 1;
                }
            }

            // Set Resume Flag (RF) 
            ExceptionInfo->ContextRecord->EFlags |= (1 << 16); 
            
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void ApplyDr0ToAllThreads(uintptr_t addr) {
    DWORD pid = GetCurrentProcessId();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te;
        te.dwSize = sizeof(THREADENTRY32);
        if (Thread32First(hSnapshot, &te)) {
            do {
                if (te.th32OwnerProcessID == pid && te.th32ThreadID != GetCurrentThreadId()) {
                    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, te.th32ThreadID);
                    if (hThread) {
                        CONTEXT ctx = {0};
                        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
                        
                        SuspendThread(hThread);
                        if (GetThreadContext(hThread, &ctx)) {
                            // Only set if not already set, to avoid redundant resets
                            if (ctx.Dr0 != addr) {
                                ctx.Dr0 = addr;
                                ctx.Dr7 |= 1; 
                                SetThreadContext(hThread, &ctx);
                            }
                        }
                        ResumeThread(hThread);
                        CloseHandle(hThread);
                    }
                }
            } while (Thread32Next(hSnapshot, &te));
        }
        CloseHandle(hSnapshot);
    }
}

DWORD WINAPI InitializeIL2CPPHook(LPVOID lpParam) {
    LOG_INFO("[IL2CPP Hook] VEH Thread started. Waiting for GameAssembly.dll...");

    HMODULE hGameAssembly = nullptr;
    while (!(hGameAssembly = GetModuleHandleW(L"GameAssembly.dll"))) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    LOG_INFO("[IL2CPP Hook] GameAssembly loaded at %p", hGameAssembly);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    uintptr_t targetRVA = 0x7952D0;
    g_targetFuncAddr = (uintptr_t)hGameAssembly + targetRVA;
    
    LOG_INFO("[IL2CPP Hook] Target function address: %p", (void*)g_targetFuncAddr);

    void* handle = AddVectoredExceptionHandler(1, VehHandler);
    if (handle) {
        LOG_INFO("[IL2CPP Hook] VEH Exception Handler registered successfully.");
    } else {
        LOG_INFO("[IL2CPP Hook] Failed to register VEH Handler.");
        return 0;
    }

    while (true) {
        ApplyDr0ToAllThreads(g_targetFuncAddr);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}
