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

        //--------------------------------------------------------------------------------------------------------------

        //-- CConfirmEmail ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CConfirmEmail: public CApostolModule {
        private:

            CString m_Token;
            CString m_Agent;
            CString m_Host;

            CDateTime m_AuthDate;

            CStringListPairs m_Profiles;

            int m_HeartbeatInterval;

            void Authentication();

            void InitMethods() override;

            static CHTTPReply::CStatusType ErrorCodeToStatus(int ErrorCode);
            static int CheckError(const CJSON &Json, CString &ErrorMessage, bool RaiseIfError = false);

            void RedirectConfirm(CHTTPServerConnection *AConnection, const CString &Result, const CString &Message);
            void RedirectError(CHTTPServerConnection *AConnection, int ErrorCode, const CString &Error, const CString &Message);

            void ConfirmEmail(CHTTPServerConnection *AConnection, const CString &Payload);

            static void InitConfig(const CIniFile &IniFile, const CString &Profile, CStringList &Config);

        protected:

            void DoError(const Delphi::Exception::Exception &E);

            void DoGet(CHTTPServerConnection *AConnection) override;

        public:

            explicit CConfirmEmail(CModuleProcess *AProcess);

            ~CConfirmEmail() override = default;

            static class CConfirmEmail *CreateModule(CModuleProcess *AProcess) {
                return new CConfirmEmail(AProcess);
            }

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
