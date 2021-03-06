﻿/*************************************************************************
 *                                                                       *
 *  I|*j^3Cl|a   "+!*%                  qt          Nd   gW              *
 *  l]{y+l?MM*  !#Wla\NNP               NW          MM   I|              *
 *        PW    ?E|    tWg              Wg  sC!     AW           ~@v~    *
 *       NC     ?M!    yN|  WW     MK   MW@K1Y%M@   RM   #Q    QP@tim    *
 *     CM|      |WQCljAE|   MD     Mg   RN     cM~  NM   WQ   MQ         *
 *    #M        aQ?         MW     M3   Mg      Q(  HQ   YR  IM|         *
 *   Dq         {Ql         MH    iMX   Mg     MM   QP   QM   Eg         *
 * !EWNaPRag2$  +M"          $WNaHaN%   MQE$%EXW    QQ   CM    %M%a$D    *
 *                                                                       *
 *               Website: https://github.com/zpublic/zpublic             *
 *                                                                       *
 ************************************************************************/

/**
 * @file
 * @brief 系统版本信息
 */


#pragma once
#include "win_utils_header.h"
#include "register.hpp"

namespace zl
{
namespace WinUtils
{
    typedef void (WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO);
    typedef BOOL (WINAPI *LPFN_IsWow64Process) (HANDLE, PBOOL);

    class ZLSystemVersion
    {
    public:
        ZLSystemVersion() {}
        ~ZLSystemVersion() {}

    public:
        enum enumSystemVersion
        {
            enumSystemVersionNone       = 0,
            enumSystemVersionWin9X      = 1,
            enumSystemVersionWinMe      = 2,
            enumSystemVersionWin2000    = 3,
            enumSystemVersionWinNT      = 4,
            enumSystemVersionWin2003    = 5,
            enumSystemVersionWinXp      = 6,
            enumSystemVersionVista      = 7,
            enumSystemVersionWin7       = 8,
            enumSystemVersionWin2008    = 9,
            enumSystemVersionWin8       = 10,
        };

        enum enumProcessorArchitecture
        {
            enumProcessorArchitectureNone,
            enumProcessorArchitecture32Bit,    // 32 CPU
            enumProcessorArchitecture64Bit,    // 64 CPU
        };

        static enumSystemVersion GetSystemVersion()
        {
            static enumSystemVersion OsPlatform = enumSystemVersionNone;
            if (OsPlatform == enumSystemVersionNone)
            {
                OsPlatform = GetSystemVersionImpl();
            }
            return OsPlatform;
        }

        static enumProcessorArchitecture GetProcessorArchitecture()
        {
            static enumProcessorArchitecture emProcArch = enumProcessorArchitectureNone;
            if (enumProcessorArchitectureNone == emProcArch)
            {
                emProcArch = enumProcessorArchitecture32Bit;
                LPFN_GetNativeSystemInfo pGNSI = (LPFN_GetNativeSystemInfo)::GetProcAddress(
                    ::GetModuleHandleW(L"kernel32.dll"),
                    "GetNativeSystemInfo");
                if (NULL != pGNSI)
                {
                    SYSTEM_INFO si = {0};
                    pGNSI(&si);
                    if (PROCESSOR_ARCHITECTURE_IA64 == si.wProcessorArchitecture ||
                        PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture ||
                        PROCESSOR_ARCHITECTURE_ALPHA64 == si.wProcessorArchitecture)
                    {
                        emProcArch = enumProcessorArchitecture64Bit;
                    }
                }
            }

            return emProcArch;
        }

        static BOOL IsVistaOrLater()
        {
            BOOL bRet = FALSE;
            enumSystemVersion emSystemVersion = GetSystemVersion();

            if ( emSystemVersion == enumSystemVersionVista   ||
                emSystemVersion == enumSystemVersionWin7    ||
                emSystemVersion == enumSystemVersionWin2008 ||
                emSystemVersion == enumSystemVersionWin8 )
            {
                bRet = TRUE;
            }
            return bRet;
        }

        static BOOL GetSystemVersion(DWORD &dwMarjorVersion, DWORD &dwMinorVersion)
        {
            BOOL bReturn = FALSE;
            bReturn = GetSystemVersionByReg(dwMarjorVersion, dwMinorVersion);
            if (!bReturn)
            {
                bReturn = GetSystemVersionByApi(dwMarjorVersion, dwMinorVersion);
            }
            return bReturn;
        }

        static BOOL IsWow64System()
        {
            static int nWow64 = -1;
            if (nWow64 == -1)
            {
                BOOL bIsWow64 = FALSE;

                LPFN_IsWow64Process pfn_IsWow64Process = (LPFN_IsWow64Process)GetProcAddress(
                    ::GetModuleHandle(_T("kernel32")), 
                    "IsWow64Process");

                if (pfn_IsWow64Process)
                {
                    if (!pfn_IsWow64Process(::GetCurrentProcess(), &bIsWow64))
                    {
                        bIsWow64 = FALSE;
                    }
                }
                nWow64 = bIsWow64;
            }
            return (BOOL)nWow64;
        }

    private:
        static enumSystemVersion GetSystemVersionImpl()
        {
            BOOL bRetCode = FALSE;
            enumSystemVersion OsPlatform = enumSystemVersionNone;

            do 
            {
                OSVERSIONINFOEX osvi;
                ::ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
                osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
                bRetCode = ::GetVersionEx((OSVERSIONINFO *)&osvi);
                if (FALSE == bRetCode)
                {
                    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                    bRetCode = ::GetVersionEx((OSVERSIONINFO *)&osvi);
                }
                if (bRetCode == FALSE)
                {
                    break;
                }

                switch(osvi.dwPlatformId)
                {
                case VER_PLATFORM_WIN32_NT:
                    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
                        OsPlatform = (osvi.wProductType == VER_NT_WORKSTATION) ? enumSystemVersionWin7 : enumSystemVersionWin2008;
                    else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
                        OsPlatform = (osvi.wProductType == VER_NT_WORKSTATION) ? enumSystemVersionVista : enumSystemVersionWin2008;
                    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
                        OsPlatform = enumSystemVersionWin2003;
                    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
                        OsPlatform = enumSystemVersionWinXp;
                    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
                        OsPlatform = enumSystemVersionWin2000;
                    else if (osvi.dwMajorVersion <= 4)
                        OsPlatform = enumSystemVersionWinNT;
                    else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2 && osvi.wProductType == VER_NT_WORKSTATION)
                        OsPlatform = enumSystemVersionWin8;
                    break;
                case VER_PLATFORM_WIN32_WINDOWS:
                    if (((osvi.dwBuildNumber >> 16) & 0x0000FFFF) < 0x045A)
                        OsPlatform = enumSystemVersionWin9X;
                    else 
                        OsPlatform = enumSystemVersionWinMe;
                    break;
                default:
                    OsPlatform = enumSystemVersionNone;
                    break;
                }
            } while (FALSE);
            return OsPlatform;
        }

        static BOOL GetSystemVersionByReg(DWORD &dwMarjorVersion, DWORD &dwMinorVersion)
        {
            BOOL bReturn = FALSE;
            zl::WinUtils::ZLRegister r;
            CString cstrVer;
            CString cstrMarjorVer;
            CString cstrMinorVer;

            do
            {
                if (!r.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")))
                {
                    break;
                }
                if (!r.Read(_T("CurrentVersion"), cstrVer))
                {
                    break;
                }

                int nPos = cstrVer.Find(_T("."));
                if (nPos == -1)
                {
                    break;
                }

                cstrMarjorVer = cstrVer.Left(nPos);
                cstrMinorVer = cstrVer.Mid(nPos + 1);
                if (cstrMarjorVer.IsEmpty() || cstrMinorVer.IsEmpty())
                {
                    break;
                }

                dwMarjorVersion = _tcstol(cstrMarjorVer, NULL, 10);
                dwMinorVersion = _tcstol(cstrMinorVer, NULL, 10);
                bReturn = TRUE;

            } while (FALSE);
            return bReturn;
        }

        static BOOL GetSystemVersionByApi(DWORD &dwMarjorVersion, DWORD &dwMinorVersion)
        {
            BOOL bReturn = FALSE;
            enumSystemVersion OsPlatform = enumSystemVersionNone;

            OSVERSIONINFOEX osvi;
            ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

            bReturn = ::GetVersionEx((OSVERSIONINFO *)&osvi);
            if (!bReturn)
            {
                osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                bReturn = ::GetVersionEx((OSVERSIONINFO *)&osvi);
            }
            if (bReturn == FALSE)
            {
                goto Exit0;
            }

            dwMarjorVersion = osvi.dwMajorVersion;
            dwMinorVersion = osvi.dwMinorVersion;

        Exit0:
            return bReturn;
        }
    };

}
}