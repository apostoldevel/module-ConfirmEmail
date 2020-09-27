/*++

Program name:

  Apostol Web Service

Module Name:

  ConfirmEmail.cpp

Notices:

  Module: Confirm email addresses

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

//----------------------------------------------------------------------------------------------------------------------

#include "Core.hpp"
#include "ConfirmEmail.hpp"
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Apostol {

    namespace Workers {

        //--------------------------------------------------------------------------------------------------------------

        //-- CConfirmEmail ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CConfirmEmail::CConfirmEmail(CModuleProcess *AProcess) : CApostolModule(AProcess,
                "confirm email", "worker/ConfirmEmail") {

            CConfirmEmail::InitMethods();
            m_CheckDate = Now();
            m_HeartbeatInterval = 5;
            m_Native = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::InitMethods() {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            m_pMethods->AddObject(_T("GET")    , (CObject *) new CMethodHandler(true , [this](auto && Connection) { DoGet(Connection); }));
            m_pMethods->AddObject(_T("OPTIONS"), (CObject *) new CMethodHandler(false, [this](auto && Connection) { MethodNotAllowed(Connection); }));
            m_pMethods->AddObject(_T("HEAD")   , (CObject *) new CMethodHandler(false, [this](auto && Connection) { MethodNotAllowed(Connection); }));
            m_pMethods->AddObject(_T("POST")   , (CObject *) new CMethodHandler(false, [this](auto && Connection) { MethodNotAllowed(Connection); }));
            m_pMethods->AddObject(_T("PUT")    , (CObject *) new CMethodHandler(false, [this](auto && Connection) { MethodNotAllowed(Connection); }));
            m_pMethods->AddObject(_T("DELETE") , (CObject *) new CMethodHandler(false, [this](auto && Connection) { MethodNotAllowed(Connection); }));
            m_pMethods->AddObject(_T("TRACE")  , (CObject *) new CMethodHandler(false, [this](auto && Connection) { MethodNotAllowed(Connection); }));
            m_pMethods->AddObject(_T("PATCH")  , (CObject *) new CMethodHandler(false, [this](auto && Connection) { MethodNotAllowed(Connection); }));
            m_pMethods->AddObject(_T("CONNECT"), (CObject *) new CMethodHandler(false, [this](auto && Connection) { MethodNotAllowed(Connection); }));
#else
            m_pMethods->AddObject(_T("GET")    , (CObject *) new CMethodHandler(true , std::bind(&CConfirmEmail::DoGet, this, _1)));
            m_pMethods->AddObject(_T("OPTIONS"), (CObject *) new CMethodHandler(false, std::bind(&CConfirmEmail::MethodNotAllowed, this, _1)));
            m_pMethods->AddObject(_T("HEAD")   , (CObject *) new CMethodHandler(false, std::bind(&CConfirmEmail::MethodNotAllowed, this, _1)));
            m_pMethods->AddObject(_T("POST")   , (CObject *) new CMethodHandler(false, std::bind(&CConfirmEmail::MethodNotAllowed, this, _1)));
            m_pMethods->AddObject(_T("PUT")    , (CObject *) new CMethodHandler(false, std::bind(&CConfirmEmail::MethodNotAllowed, this, _1)));
            m_pMethods->AddObject(_T("DELETE") , (CObject *) new CMethodHandler(false, std::bind(&CConfirmEmail::MethodNotAllowed, this, _1)));
            m_pMethods->AddObject(_T("TRACE")  , (CObject *) new CMethodHandler(false, std::bind(&CConfirmEmail::MethodNotAllowed, this, _1)));
            m_pMethods->AddObject(_T("PATCH")  , (CObject *) new CMethodHandler(false, std::bind(&CConfirmEmail::MethodNotAllowed, this, _1)));
            m_pMethods->AddObject(_T("CONNECT"), (CObject *) new CMethodHandler(false, std::bind(&CConfirmEmail::MethodNotAllowed, this, _1)));
#endif
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::LoadConfig(const CString &FileName) {

            const CString Prefix(Config()->Prefix());
            CString ConfigFile(FileName);

            if (!path_separator(ConfigFile.front())) {
                ConfigFile = Prefix + ConfigFile;
            }

            LPCTSTR lpSectionName = "main";
            LPCTSTR lpDefaultUri = "/verification/email";

            if (FileExists(ConfigFile.c_str())) {
                CIniFile IniFile(ConfigFile.c_str());
                IniFile.OnIniFileParseError(OnIniFileParseError);

                m_Native = IniFile.ReadBool(lpSectionName, "native", false);
                m_Redirect = IniFile.ReadString(lpSectionName, "redirect_uri", lpDefaultUri);
                m_RedirectError = IniFile.ReadString(lpSectionName, "redirect_uri_error", m_Redirect);

            } else {

                m_Native = false;
                m_Redirect = lpDefaultUri;
                m_RedirectError = lpDefaultUri;

                Log()->Error(APP_LOG_EMERG, 0, APP_FILE_NOT_FOUND, ConfigFile.c_str());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPReply::CStatusType CConfirmEmail::ErrorCodeToStatus(int ErrorCode) {
            CHTTPReply::CStatusType Status = CHTTPReply::ok;

            if (ErrorCode != 0) {
                switch (ErrorCode) {
                    case 401:
                        Status = CHTTPReply::unauthorized;
                        break;

                    case 403:
                        Status = CHTTPReply::forbidden;
                        break;

                    case 404:
                        Status = CHTTPReply::not_found;
                        break;

                    case 500:
                        Status = CHTTPReply::internal_server_error;
                        break;

                    default:
                        Status = CHTTPReply::bad_request;
                        break;
                }
            }

            return Status;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CConfirmEmail::CheckError(const CJSON &Json, CString &ErrorMessage, bool RaiseIfError) {
            int ErrorCode = 0;

            if (Json.HasOwnProperty(_T("error"))) {
                const auto& error = Json[_T("error")];

                if (error.HasOwnProperty(_T("code"))) {
                    ErrorCode = error[_T("code")].AsInteger();
                } else {
                    ErrorCode = 40000;
                }

                if (error.HasOwnProperty(_T("message"))) {
                    ErrorMessage = error[_T("message")].AsString();
                } else {
                    ErrorMessage = _T("Invalid request.");
                }

                if (RaiseIfError)
                    throw EDBError(ErrorMessage.c_str());

                if (ErrorCode >= 10000)
                    ErrorCode = ErrorCode / 100;

                if (ErrorCode < 0)
                    ErrorCode = 400;
            }

            return ErrorCode;
        }

        void CConfirmEmail::RedirectConfirm(CHTTPServerConnection *AConnection, const CString &Result, const CString &Message) {

            CString Location(m_Redirect);

            Location << "?result=" << Result;
            Location << "&message=" << CHTTPServer::URLEncode(Message);

            Redirect(AConnection, Location, true);

            Log()->Error(APP_LOG_EMERG, 0, _T("RedirectConfirm: %s"), Message.c_str());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::RedirectError(CHTTPServerConnection *AConnection, int ErrorCode, const CString &Error, const CString &Message) {

            CString ErrorLocation(m_RedirectError);

            ErrorLocation << "?code=" << ErrorCode;
            ErrorLocation << "&error=" << Error;
            ErrorLocation << "&message=" << CHTTPServer::URLEncode(Message);

            Redirect(AConnection, ErrorLocation, true);

            Log()->Error(APP_LOG_EMERG, 0, _T("RedirectError: %s"), Message.c_str());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::Authorize() {

            auto OnExecuted = [this](CPQPollQuery *APollQuery) {

                CPQResult *Result;
                CStringList SQL;

                try {
                    for (int I = 0; I < APollQuery->Count(); I++) {
                        Result = APollQuery->Results(I);

                        if (Result->ExecStatus() != PGRES_TUPLES_OK)
                            throw Delphi::Exception::EDBError(Result->GetErrorMessage());

                        const CJSON Json(Result->GetValue(0, 0));

                        m_Auth.Session = Json["session"].AsString();
                        m_Auth.Secret = Json["secret"].AsString();
                        m_Auth.Token = Json["access_token"].AsString();

                        m_CheckDate = Now() + (CDateTime) 23 * 60 / MinsPerDay;
                    }
                } catch (Delphi::Exception::Exception &E) {
                    m_CheckDate = Now() + (CDateTime) m_HeartbeatInterval * 3 / SecsPerDay;
                    Log()->Error(APP_LOG_EMERG, 0, E.what());
                }
            };

            auto OnException = [this](CPQPollQuery *APollQuery, const Delphi::Exception::Exception &E) {
                m_CheckDate = Now() + (CDateTime) m_HeartbeatInterval * 3 / SecsPerDay;
                Log()->Error(APP_LOG_EMERG, 0, E.what());
            };

            CString Application("web");

            const auto &Providers = Server().Providers();
            const auto &Provider = Providers.Default().Value();

            m_Auth.Username = Provider.ClientId(Application);
            m_Auth.Password = Provider.Secret(Application);

            m_Auth.Agent = "Confirm email";
            m_Auth.Host = CApostolModule::GetIPByHostName(CApostolModule::GetHostName());

            CStringList SQL;

            SQL.Add(CString().Format("SELECT * FROM daemon.token(%s, %s, '%s'::jsonb, %s, %s);",
                                     PQQuoteLiteral(m_Auth.Username).c_str(),
                                     PQQuoteLiteral(m_Auth.Password).c_str(),
                                     R"({"grant_type": "client_credentials"})",
                                     PQQuoteLiteral(m_Auth.Agent).c_str(),
                                     PQQuoteLiteral(m_Auth.Host).c_str()
            ));

            try {
                ExecSQL(SQL, nullptr, OnExecuted, OnException);
            } catch (Delphi::Exception::Exception &E) {
                m_CheckDate = Now() + (CDateTime) m_HeartbeatInterval * 3 / SecsPerDay;
                Log()->Error(APP_LOG_EMERG, 0, E.what());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::ConfirmEmail(CHTTPServerConnection *AConnection, const CString &Payload) {

            auto OnExecuted = [this, AConnection](CPQPollQuery *APollQuery) {

                CPQResult *LResult;

                try {
                    for (int I = 0; I < APollQuery->Count(); I++) {
                        LResult = APollQuery->Results(I);

                        if (LResult->ExecStatus() != PGRES_TUPLES_OK)
                            throw Delphi::Exception::EDBError(LResult->GetErrorMessage());

                        CString ErrorMessage;
                        const CJSON Payload(LResult->GetValue(0, 0));
                        auto Status = ErrorCodeToStatus(CheckError(Payload, ErrorMessage));
                        if (Status == CHTTPReply::ok) {
                            const auto &Result = Payload["result"].AsString();
                            const auto &Message = Payload["message"].AsString();
                            RedirectConfirm(AConnection, Result, Message);
                        } else {
                            RedirectError(AConnection, Status, "unsuccessful", ErrorMessage);
                        }
                    }
                } catch (Delphi::Exception::Exception &E) {
                    RedirectError(AConnection, CHTTPReply::bad_request, "invalid_request", E.what());
                }
            };

            auto OnException = [this, AConnection](CPQPollQuery *APollQuery, const Delphi::Exception::Exception &E) {
                RedirectError(AConnection, CHTTPReply::internal_server_error, "server_error", E.what());
            };

            CStringList SQL;

            SQL.Add(CString().Format("SELECT * FROM daemon.fetch(%s, '%s', '%s'::jsonb, %s, %s);",
                                     PQQuoteLiteral(m_Auth.Token).c_str(),
                                     "/verification/email/confirm",
                                     Payload.c_str(),
                                     PQQuoteLiteral(m_Auth.Agent).c_str(),
                                     PQQuoteLiteral(m_Auth.Host).c_str()
            ));

            try {
                ExecSQL(SQL, AConnection, OnExecuted, OnException);
            } catch (Delphi::Exception::Exception &E) {
                RedirectError(AConnection, CHTTPReply::internal_server_error, "server_error", E.what());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::DoError(const Delphi::Exception::Exception &E) {
            m_Auth.Clear();
            m_CheckDate = Now();
            Log()->Error(APP_LOG_EMERG, 0, E.what());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::DoGet(CHTTPServerConnection *AConnection) {
            auto LRequest = AConnection->Request();

            CString LPath(LRequest->Location.pathname);

            // Request path must be absolute and not contain "..".
            if (LPath.empty() || LPath.front() != '/' || LPath.find(_T("..")) != CString::npos) {
                AConnection->SendStockReply(CHTTPReply::bad_request);
                return;
            }

            CStringList LRouts;
            SplitColumns(LPath, LRouts, '/');

            if (LRouts.Count() < 3) {
                AConnection->SendStockReply(CHTTPReply::not_found);
                return;
            }

            if (m_Native) {
                const auto& Code = LRouts[2];
                ConfirmEmail(AConnection, CString().Format(R"({"code": "%s"})", Code.c_str()));
            } else {
                LPath.Clear();

                for (int I = 0; I < 2; ++I) {
                    LPath.Append('/');
                    LPath.Append(LRouts[I]);
                }
                LPath.Append("/index.html");

                SendResource(AConnection, LPath);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::Initialization(CModuleProcess *AProcess) {
            CApostolModule::Initialization(AProcess);
            LoadConfig(Config()->IniFile().ReadString(SectionName().c_str(), "config", "conf/confirm_email.conf"));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CConfirmEmail::Heartbeat() {
            auto now = Now();
            if ((now >= m_CheckDate)) {
                if (m_Auth.Session.IsEmpty()) {
                    m_CheckDate = now + (CDateTime) m_HeartbeatInterval / SecsPerDay;
                    Authorize();
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CConfirmEmail::Enabled() {
            if (m_ModuleStatus == msUnknown)
                m_ModuleStatus = Config()->IniFile().ReadBool(SectionName().c_str(), "enable", false) ? msEnabled : msDisabled;
            return m_ModuleStatus == msEnabled;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CConfirmEmail::CheckLocation(const CLocation &Location) {
            return Location.pathname.SubString(0, 15) == _T("/confirm/email/");
        }
    }
}
}