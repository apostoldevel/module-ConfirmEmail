/*++

Program name:

  Apostol Web Service

Module Name:

  ConfirmEmail.hpp

Notices:

  Module: Confirm email addresses

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef APOSTOL_CONFIRM_EMAIL_HPP
#define APOSTOL_CONFIRM_EMAIL_HPP
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Apostol {

    namespace Workers {

        struct CAuth {
            CString Username;
            CString Password;

            CString Agent;
            CString Host;

            CString Session;
            CString Secret;
            CString Token;

            void Clear() {
                Username.Clear();
                Password.Clear();

                Agent.Clear();
                Host.Clear();

                Session.Clear();
                Secret.Clear();
                Token.Clear();
            }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CConfirmEmail ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CConfirmEmail: public CApostolModule {
        private:

            CAuth m_Auth;

            CDateTime m_CheckDate;

            TPairs<CStringList> m_Profiles;

            CHTTPProxyManager m_ProxyManager;

            int m_HeartbeatInterval;

            void InitMethods() override;

            void Authorize();

            static CHTTPReply::CStatusType ErrorCodeToStatus(int ErrorCode);
            static int CheckError(const CJSON &Json, CString &ErrorMessage, bool RaiseIfError = false);

            void ConfirmEmail(CHTTPServerConnection *AConnection, const CString &Payload);

        protected:

            void DoError(const Delphi::Exception::Exception &E);

            void DoGet(CHTTPServerConnection *AConnection) override;

        public:

            explicit CConfirmEmail(CModuleProcess *AProcess);

            ~CConfirmEmail() override = default;

            static class CConfirmEmail *CreateModule(CModuleProcess *AProcess) {
                return new CConfirmEmail(AProcess);
            }

            void RedirectConfirm(CHTTPServerConnection *AConnection, const CString &Result, const CString &Message);
            void RedirectError(CHTTPServerConnection *AConnection, int ErrorCode, const CString &Error, const CString &Message);

            static void InitConfig(const CIniFile &IniFile, const CString &Profile, CStringList &Config);

            void Initialization(CModuleProcess *AProcess) override;

            void Heartbeat() override;

            bool Enabled() override;
            bool CheckLocation(const CLocation &Location) override;

        };
    }
}

using namespace Apostol::Workers;
}
#endif //APOSTOL_CONFIRM_EMAIL_HPP
