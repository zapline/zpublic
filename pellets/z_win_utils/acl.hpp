/*************************************************************************
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
 * @brief 访问控制列表相关
 */


#pragma once
#include "win_utils_header.h"
#include <Aclapi.h>
#pragma comment(lib,"Advapi32")

namespace zl
{
namespace WinUtils
{

    class ZLAcl
    {
    public:
        ZLAcl()
            : pDacl_(NULL)
            , pDescriptor_(NULL)
            , pSacl_(NULL)
            , enumObjectType_(SE_UNKNOWN_OBJECT_TYPE)
            , SecurityInfo_(NULL)
            , bIsOpen_(FALSE)
        {
        }

        ~ZLAcl()
        {
            Close();
        }

    public:
        BOOL Open(const CString& cstrObjcetName,
            SE_OBJECT_TYPE enumObjectType,
            SECURITY_INFORMATION SecurityInfo = DACL_SECURITY_INFORMATION)
        {
            if (cstrObjcetName.IsEmpty())
            {
                return FALSE;
            }
            if (::GetNamedSecurityInfo(cstrObjcetName,
                enumObjectType,
                SecurityInfo,
                NULL,
                NULL,
                &pDacl_,
                &pSacl_,
                &pDescriptor_) != ERROR_SUCCESS)
            {
                return FALSE;
            }
            cstrObjectName_ = cstrObjcetName;
            enumObjectType_ = enumObjectType;
            SecurityInfo_ = SecurityInfo;
            bIsOpen_ = TRUE;
            return TRUE;
        }

        BOOL SetSecurity(const CString& cstrUserName,
            DWORD dwAccessPermissions = KEY_ALL_ACCESS,
            ACCESS_MODE enumAccessMode = SET_ACCESS,
            DWORD dwInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT)
        {
            if (!bIsOpen_
                || cstrUserName.IsEmpty())
            {
                return FALSE;
            }
            BOOL bRet = FALSE;
            PACL pNewDacl = NULL;
            EXPLICIT_ACCESS explicitacc = {0};
            do 
            {
                ::BuildExplicitAccessWithName(&explicitacc,
                    (LPTSTR)(LPCTSTR)cstrUserName,
                    dwAccessPermissions,
                    enumAccessMode,
                    dwInheritance);
                if (::SetEntriesInAcl(1,
                    &explicitacc,
                    pDacl_,
                    &pNewDacl) != ERROR_SUCCESS)
                {
                    break;
                }
                if (pDacl_)
                {
                    ::LocalFree(pDacl_);
                    pDacl_ = NULL;
                }

                pDacl_ = pNewDacl;
                if (::SetNamedSecurityInfo((LPTSTR)(LPCTSTR)cstrObjectName_,
                    enumObjectType_,
                    SecurityInfo_,
                    NULL,
                    NULL,
                    pDacl_,
                    pSacl_) == ERROR_SUCCESS)
                {
                    bRet = TRUE;
                }
            } while (FALSE);
            return bRet;
        }

        void Close()
        {
            if (pDacl_)
            {
                ::LocalFree(pDacl_);
                pDacl_ = NULL;
            }
            if (pSacl_)
            {
                ::LocalFree(pSacl_);
                pSacl_ = NULL;
            }
            if (pDescriptor_)
            {
                ::LocalFree(pDescriptor_);
                pDescriptor_ = NULL;
            }
            cstrObjectName_.Empty();
            enumObjectType_ = SE_UNKNOWN_OBJECT_TYPE;
        }

    private:
        BOOL bIsOpen_;
        PACL pDacl_;
        PACL pSacl_;
        CString cstrObjectName_;
        SE_OBJECT_TYPE enumObjectType_;
        SECURITY_INFORMATION SecurityInfo_;
        PSECURITY_DESCRIPTOR pDescriptor_;
    };
}
}