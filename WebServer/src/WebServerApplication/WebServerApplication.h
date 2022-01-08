#ifndef XPLATFORM_WEB_SERVER_APPLICATION
#define XPLATFORM_WEB_SERVER_APPLICATION

#include <XPlatform.Utils/Application/Application.h>

#include <XPlatform.net/Net.h>
#include <XPlatform.net/XP_ISocket.h>
#include <XPlatform.net/SSL/SSL.h>
#include <XPlatform.net/SSL/SSLCtx.h>

#include <XPlatform.net/XP_IUdpSocket.h>

#include <thread>
#include <mutex>

namespace XPlatform {
	namespace Net {
		class WebServerApplication : public XPlatform::XP_IApplication {
			bool m_IsApplicationRunning = true;
			bool m_bIPv6 = false;
			bool m_bIPFromHostnet = false;
			bool m_bEnableLogging = true;
			bool m_bIsSSLEnabled = false;
			uint16_t m_nPort = 4642;
			XPlatform::Net::XP_ISocket* m_pxiSocket;
			XPlatform::Net::SSL::Ctx* m_pCtx;
			void(*m_pPluginHandleRequest)(const std::string& m_srRequest, const std::string& m_srSiteFolder, XPlatform::Net::XP_ISocket* m_pxiClient);
			std::vector<std::thread> m_vThreadPool;
			std::string m_sIP = "127.0.0.1";
			std::string m_sCertificateFilePath;
			std::string m_sPrivateKeyFilePath;
			std::string m_sPrivateKeyFilePass;
			std::string m_sSiteFolder;

			void ListenThread();
			void ClientHandler(XPlatform::Net::XP_ISocket* m_pxiClientSocket);

			void HandleRequest(const std::string& Request, XPlatform::Net::XP_ISocket* m_pxiClientSocket);

			bool HandleArgs(int argc, char* argv[]);
			bool BuildSocket();

			// Utils
			void SplitStringToVector(std::vector<std::string>& m_rOut, const std::string& m_rString);

		public:

			int Init(int argc, char* argv[]);

			int Run();

		};
	}
}

#endif