#include "WebServerApplication.h"

#include <thread>
#include <mutex>

#include <iostream>
#include <sstream>

#include <cassert>

void XPlatform::Net::WebServerApplication::ListenThread(){
	
	std::cout << "[server] listening on ";
	std::string Proto = m_bIsSSLEnabled ? "https://" : "http://";
	std::cout << Proto;
	if (!m_bIPv6)
		std::cout << m_sIP ;
	else
		std::cout << "[" << m_sIP << "]";

	std::cout << ":" << m_nPort << std::endl;

	while (m_IsApplicationRunning) {
		if (m_pxiSocket->Select(10) == XPlatform::Net::XPlatformNetResult::XPLATFORM_NET_RESULT_SELECT_SUCCESS) {
			XPlatform::Net::XP_ISocket* m_pxiClient = m_pxiSocket->Accept();
			

			if (m_pxiClient == NULL) continue;
			m_vThreadPool.push_back(std::thread(&XPlatform::Net::WebServerApplication::ClientHandler, this, m_pxiClient));
		}
	}
}

void XPlatform::Net::WebServerApplication::ClientHandler(XPlatform::Net::XP_ISocket* m_pxiClientSocket){
	char RecvBuff[1024] = "\0";
	std::memset(RecvBuff, 0, 1024);

	uint32_t BytesReaded = m_pxiClientSocket->Recv(RecvBuff, 1024);

	if(m_bEnableLogging)
		printf("new request from [%s:%i]{Bytes readed: %i}:\n %s", m_pxiClientSocket->GetEndPoint().GetIP(), m_pxiClientSocket->GetEndPoint().GetPort(), BytesReaded, RecvBuff);

	HandleRequest(RecvBuff, m_pxiClientSocket);

	m_pxiClientSocket->Close();
	m_pxiClientSocket->Release();
}

void XPlatform::Net::WebServerApplication::HandleRequest(const std::string& Request, XPlatform::Net::XP_ISocket* m_pxiClientSocket){
	assert(m_pPluginHandleRequest != NULL);
	m_pPluginHandleRequest(Request, m_sSiteFolder, m_pxiClientSocket);
}

bool XPlatform::Net::WebServerApplication::HandleArgs(int argc, char* argv[]){
	for (std::ptrdiff_t i = 0; i < argc; i++) {
		if (strcmp("-h", argv[i]) == 0) {
			std::cout << "SSL:" << std::endl;
			std::cout << "-SSL :Enable ssl!" << std::endl;
			std::cout << "-PrivateKey <String>:Setup private key file path!" << std::endl;
			std::cout << "-Certificate <String>:Setup certificate file path!" << std::endl;
			std::cout << "-PrivateKeyPass <String>:Setup private key password!" << std::endl;
			std::cout << "General:" << std::endl;
			std::cout << "-l <f:t>:Enable or disable request logging!" << std::endl;
			std::cout << "-P <Num>:Setup port!" << std::endl;
			std::cout << "-IP <String>:Setup IP!" << std::endl;
			std::cout << "-IPv6 :enable IPv6!" << std::endl;
			std::cout << "-f <String>: Setup web site folder!" << std::endl;
			std::cout << "-LoadPlugin <Path:String>: Load plugin form Path!" << std::endl;
			std::cout << "-SetRequestHandleAddr <ExtId:Num> <FunctionName:String>: Setup request handler address!" << std::endl;

			return false;
		}
		else if (strcmp("-SSL", argv[i]) == 0) {
			m_bIsSSLEnabled = true;
		}
		else if (strcmp("-PrivateKey", argv[i]) == 0) {
			i++;
			m_sPrivateKeyFilePath = argv[i];
		}
		else if (strcmp("-PrivateKeyPass", argv[i]) == 0) {
			i++;
			m_sPrivateKeyFilePass = argv[i];
		}
		else if (strcmp("-Certificate", argv[i]) == 0) {
			i++;
			m_sCertificateFilePath = argv[i];
		}
		else if (strcmp("-l", argv[i]) == 0) {
			i++;
			m_bEnableLogging = argv[i][0] == 't' ? true : false;
		}
		else if (strcmp("-P", argv[i]) == 0) {
			i++;
			m_nPort = atoi(argv[i]);
		}
		else if (strcmp("-IP", argv[i]) == 0) {
			i++;
			m_sIP = argv[i];
            std::cout << i << ")" << "m_sIP:" << m_sIP << " argv[" << i << "]:" << argv[i] << std::endl;
		}
		else if (strcmp("-IPv6", argv[i]) == 0) {
			m_bIPv6 = true;
		}
		else if (strcmp("-IPFromHostNet", argv[i]) == 0) {
			m_bIPFromHostnet = true;
		}
		else if (strcmp("-f", argv[i]) == 0) {
			i++;
			m_sSiteFolder = argv[i];
		}
		else if (strcmp("-LoadPlugin", argv[i]) == 0) {
			i++;
			XPlatform::core::XPlatformExtensionInfo PluginExtInfo{};
			PluginExtInfo.s_Name = "_PLUGIN_EXT";
			PluginExtInfo.s_Path = argv[i];
			PluginExtInfo.ExtId = m_pEngine->GetExtensionsInfoList().size();
			if (m_pEngine->LoadExtension(PluginExtInfo) != XPlatform::Api::XPResult::XPLATFORM_RESULT_SUCCESS)std::cout << "[server]: failed to load plugin!" << std::endl;
		}
		else if (strcmp("-SetRequestHandleAddr", argv[i]) == 0) {
			i++;
			const char* m_scExtId = argv[i];
			if (m_scExtId == NULL)continue;

			uint32_t ExtId = atoi(m_scExtId);

			if (m_pEngine->GetExtensionsInfoList().size() <= ExtId) {
				std::cout << "[server]: extension don't found!" << std::endl;
				continue;
			}

			const XPlatform::core::XPlatformExtensionModule* ExtModule = m_pEngine->GetExtensionModule(ExtId);
			if (ExtModule == NULL)
				std::cout << "[server]: extension don't found!" << std::endl;

			i++;
			void* m_pfnHandle = ExtModule->GetProc(argv[i]);
			if (m_pfnHandle == NULL) {
				std::cout << "[server]: failed to find function: '" << argv[i] << "'" << std::endl;
				continue;
			}

			m_pPluginHandleRequest = reinterpret_cast<void(*)(const std::string&, const std::string&, XPlatform::Net::XP_ISocket*)>(m_pfnHandle);
		}
	}

	if (m_bIsSSLEnabled) {
		char m_cErr = 0x0;
		if (m_sPrivateKeyFilePath.empty()) {
			m_cErr++;
			std::cout << "[server]:ssl is enabled but no private key is specified" << std::endl;
		}
		if (m_sCertificateFilePath.empty()) {
			m_cErr++;
			std::cout << "[server]:ssl is enabled but no certificate key is specified" << std::endl;
		}

		if(m_cErr > 0)
			return false;
	}

	return true;
}

bool XPlatform::Net::WebServerApplication::BuildSocket() {
	XPlatform::core::XPlatformExtensionInfo* NetExtInfo = new XPlatform::core::XPlatformExtensionInfo;

	XPlatform::Api::XPResult res = m_pEngine->GetExtensionInfo(XPLATFORM_NET_EXT_NAME, NetExtInfo);
	if (res != XPlatform::Api::XPResult::XPLATFORM_RESULT_SUCCESS) {
		std::cout << "[server]: failed to find network extension!" << std::endl;

		return false;
	}

	XPlatform::Net::XPlatformNetResult NetRes = XPlatform::Net::XPlatformNetResult::XPLATFORM_NET_RESULT_FAILED;

	if (m_bIPFromHostnet) {
		XPlatform::Net::pfn_XPlatformNetResolveHostnameToIP XPlatformNetResolveHostnameToIPs =
			reinterpret_cast<XPlatform::Net::pfn_XPlatformNetResolveHostnameToIP>(
				m_pEngine->GetExtensionModule(NetExtInfo->ExtId)->GetProc(XPlatform::Net::XPlatformNetResolveHostnameToIP)
			);

		if(XPlatformNetResolveHostnameToIPs == NULL){
			std::cout << "[server]failed to find function: " << XPlatform::Net::XPlatformNetResolveHostnameToIP << std::endl;
			return false;
		}

		const std::string& m_stIP = XPlatformNetResolveHostnameToIPs(m_sIP);
		m_sIP = m_stIP;

	}

	if (!m_bIsSSLEnabled) {
		if (m_bIPv6) {
			m_pxiSocket = reinterpret_cast<XPlatform::Net::XP_ISocket*>(
				m_pEngine->CreateExtensionClass(NetExtInfo->ExtId, XPLATFORM_EXT_NET_CLASS_ID_TCP_SOCKETv6)
				);

			if (!m_sIP.empty())
				NetRes = m_pxiSocket->Listen(XPlatform::Net::IPEndPoint6(m_sIP, m_nPort));
			else
				NetRes = m_pxiSocket->Listen(XPlatform::Net::IPEndPoint6(m_nPort));
		}
		else {
			m_pxiSocket = reinterpret_cast<XPlatform::Net::XP_ISocket*>(
				m_pEngine->CreateExtensionClass(NetExtInfo->ExtId, XPLATFORM_EXT_NET_CLASS_ID_TCP_SOCKET)
				);

			if (!m_sIP.empty())
				NetRes = m_pxiSocket->Listen(XPlatform::Net::IPEndPoint(m_sIP, m_nPort));
			else
				NetRes = m_pxiSocket->Listen(XPlatform::Net::IPEndPoint(m_nPort));
		}
	}
	else {
		XPlatform::Net::SSL::pfn_XPlatformCreateContextFunction XPlatformCreateContextFunction =
			reinterpret_cast<XPlatform::Net::SSL::pfn_XPlatformCreateContextFunction>(
				m_pEngine->GetExtensionModule(NetExtInfo->ExtId)->GetProc(XPlatform::Net::SSL::m_pXPlatformCreateContextFunctionName)
				);

		m_pCtx = XPlatformCreateContextFunction(false);

		if (!m_pCtx->SetCertificateFile(m_sCertificateFilePath, XPlatform::Net::SSL::SSLFileType::PEM)) {
			m_pCtx->Release();
			return false;
		}
		
		if (!m_sPrivateKeyFilePass.empty()) m_pCtx->SetDefualtPasswordCallbackUserData((void*)m_sPrivateKeyFilePass.data());

		if (!m_pCtx->SetPrivateKeyFile(m_sPrivateKeyFilePath, XPlatform::Net::SSL::SSLFileType::PEM)) {
			std::cout << "[server]{SetPrivateKeyFile}: Wrong password or file dont found '" << m_sPrivateKeyFilePath << std::endl;
			m_pCtx->Release();
			return false;
		}

		XPlatform::Net::SSL::pfn_XPlatformCreateSSLTCPSocketFunction XPlatformCreateSSLTCPSocketFunction;
		if (m_bIPv6) {
			XPlatformCreateSSLTCPSocketFunction =
				reinterpret_cast<XPlatform::Net::SSL::pfn_XPlatformCreateSSLTCPSocketFunction>(
					m_pEngine->GetExtensionModule(NetExtInfo->ExtId)->GetProc(XPlatform::Net::SSL::m_pXPlatformCreateSSLTCPSocketV6FunctionName)
					);

			m_pxiSocket = XPlatformCreateSSLTCPSocketFunction(m_pCtx);

			if (!m_sIP.empty())
				NetRes = m_pxiSocket->Listen(XPlatform::Net::IPEndPoint6(m_sIP, m_nPort));
			else
				NetRes = m_pxiSocket->Listen(XPlatform::Net::IPEndPoint6(m_nPort));
		}
		else {
			XPlatformCreateSSLTCPSocketFunction =
				reinterpret_cast<XPlatform::Net::SSL::pfn_XPlatformCreateSSLTCPSocketFunction>(
					m_pEngine->GetExtensionModule(NetExtInfo->ExtId)->GetProc(XPlatform::Net::SSL::m_pXPlatformCreateSSLTCPSocketFunctionName)
					);

			m_pxiSocket = XPlatformCreateSSLTCPSocketFunction(m_pCtx);

			if (!m_sIP.empty())
				NetRes = m_pxiSocket->Listen(XPlatform::Net::IPEndPoint(m_sIP, m_nPort));
			else
				NetRes = m_pxiSocket->Listen(XPlatform::Net::IPEndPoint(m_nPort));
		}
	}

	printf("[server] Net extension id:%i\n", NetExtInfo->ExtId);

	switch (NetRes)
	{
	case XPlatform::Net::XPlatformNetResult::XPLATFORM_NET_RESULT_FAILED:
		printf("[server]: failed to create socket!\n");
		return false;
		break;
	case XPlatform::Net::XPlatformNetResult::XPLATFORM_NET_RESULT_FAILED_TO_BIND_SOCKET_TO_ADDRESS:
		printf("[server]: failed to bind socket to address %s:%i!\n", m_sIP.c_str(), m_nPort);
		return false;
		break;
	case XPlatform::Net::XPlatformNetResult::XPLATFORM_NET_RESULT_FAILED_TO_LISTEN_SOCKET:
		printf("[server]: failed to listen socket!\n");
		return false;
		break;
	}

	return true;
}

void XPlatform::Net::WebServerApplication::SplitStringToVector(std::vector<std::string>& m_rOut, const std::string& m_rString){
	if (!m_rOut.empty())m_rOut.clear();
	
	std::string Word;
	std::stringstream ss(m_rString);
	while (ss >> Word) m_rOut.push_back(Word);
}

void WebServerStandardHandleRequest(const std::string& m_srRequest, const std::string& m_srSiteFolder, XPlatform::Net::XP_ISocket* m_pxiClient) {
	std::stringstream response;
	std::stringstream response_body;

	response_body << "<title>XPlatform C++ HTTP Server</title>\n"
		<< "<h1>Test page</h1>\n"
		<< "<h2>" << "addr:" << m_pxiClient->GetEndPoint().GetIP() << ":" << m_pxiClient->GetEndPoint().GetPort() << "<h2>\n"
		<< "<h2>Request headers</h2>\n"
		<< "<pre>" << m_srRequest << "</pre>\n"
		<< "<em><small>XPlatform C++ HTTP Server</small></em>\n";

	response << "HTTP/1.1 200 OK\r\n"
		<< "Version: HTTP/1.1\r\n"
		<< "Content-Type: text/html; charset=utf-8\r\n"
		<< "Content-Length: " << response_body.str().length()
		<< "\r\n\r\n"
		<< response_body.str();

	int32_t BytesSended = 0;
	BytesSended = m_pxiClient->Send(response.str().c_str(), response.str().length());
	std::cout << "[server] bytes sended:" << BytesSended << std::endl;
}

int XPlatform::Net::WebServerApplication::Init(int argc, char* argv[]){
	m_pCtx = NULL;
	
	if (!HandleArgs(argc, argv)) return -1;
	if (!BuildSocket()) return -2;

	m_pPluginHandleRequest = WebServerStandardHandleRequest;

	return 0;
}

int XPlatform::Net::WebServerApplication::Run(){

	//listen thread
	std::thread m_ListenThread(&XPlatform::Net::WebServerApplication::ListenThread, this);

	// Command handler thread
	
	std::string Input = "";
	std::vector<std::string> Command;
	while (m_IsApplicationRunning) {
		std::getline(std::cin, Input);
		if (Input.empty())continue;
		SplitStringToVector(Command, Input);
		if (Command.size() < 1) continue;

		if (Command[0] == "CmdStop") {
			m_IsApplicationRunning = false;
		}
		else if (Command[0] == "help") {
			std::cout << "CmdSetRequestHandlerAddr <ext_id> <function_name>" << std::endl;
			std::cout << "CmdShowExts" << std::endl;
			std::cout << "CmdPrintCallAddr <ext_id> <function_name>" << std::endl;
			std::cout << "CmdCall <ext_id> <function_name> <arg>" << std::endl;
			std::cout << "CmdToggleLogging" << std::endl;
			std::cout << "CmdLoadPlugin <Path>" << std::endl;
		}
		else if (Command[0] == "CmdSetRequestHandlerAddr") {
			if (Command.size() < 3) {
				std::cout << "[server]: CmdSetRequestHandlerAddr <ext_id> <function_name>" << std::endl;
				continue;
			}

			uint32_t ExtId = atoi(Command[1].c_str());

			if (m_pEngine->GetExtensionsInfoList().size() <= ExtId) {
				std::cout << "[server]: extension don't found!" << std::endl;
				continue;
			}

			const XPlatform::core::XPlatformExtensionModule* ExtModule = m_pEngine->GetExtensionModule(ExtId);
			if (ExtModule == NULL)
				std::cout << "[server]: extension don't found!" << std::endl;

			void* m_pfnHandle = ExtModule->GetProc(Command[2]);
			if (m_pfnHandle == NULL) {
				std::cout << "[server]: failed to find function: '" << Command[2] << "'" << std::endl;
				continue;
			}

			m_pPluginHandleRequest = reinterpret_cast<void(*)(const std::string&, const std::string&, XPlatform::Net::XP_ISocket*)>(m_pfnHandle);
		}
		else if (Command[0] == "CmdShowExts") {
			const std::vector<XPlatform::core::XPlatformExtensionInfo>& m_ExtList = m_pEngine->GetExtensionsInfoList();

			std::cout << "[server] extensions count = " << m_ExtList.size() << std::endl;
			for (const XPlatform::core::XPlatformExtensionInfo& ExtInfo : m_ExtList) {
				std::cout << ExtInfo.s_Name << ":" << ExtInfo.s_Path << ":" << ExtInfo.ExtId << std::endl;
			}

		}
		else if (Command[0] == "CmdCall") {
			if (Command.size() < 4) {
				std::cout << "[server]: CmdCall <ext_id> <function_name> <arg>" << std::endl;
				continue;
			}

			uint32_t ExtId = atoi(Command[1].c_str());

			if (m_pEngine->GetExtensionsInfoList().size() <= ExtId) {
				std::cout << "[server]: extension don't found!" << std::endl;
				continue;
			}

			const XPlatform::core::XPlatformExtensionModule* ExtModule = m_pEngine->GetExtensionModule(ExtId);
			if (ExtModule == NULL)
				std::cout << "[server]: extension don't found!" << std::endl;

			void(*m_pfnHandle)(const std::string & arg) = reinterpret_cast<void(*)(const std::string & arg)>(
				ExtModule->GetProc(Command[2])
				);
			if (m_pfnHandle == NULL) {
				std::cout << "[server]: failed to find function: '" << Command[2] << "'" << std::endl;
				continue;
			}

			m_pfnHandle(Command[3]);
		}
		else if (Command[0] == "CmdPrintCallAddr") {
			if (Command.size() < 3) {
				std::cout << "[server]: CmdPrintCallAddr <ext_id> <function_name>" << std::endl;
				continue;
			}

			uint32_t ExtId = atoi(Command[1].c_str());

			if (m_pEngine->GetExtensionsInfoList().size() <= ExtId) {
				std::cout << "[server]: extension don't found!" << std::endl;
				continue;
			}

			const XPlatform::core::XPlatformExtensionModule* ExtModule = m_pEngine->GetExtensionModule(ExtId);
			if (ExtModule == NULL) {
				std::cout << "[server]: extension don't found!" << std::endl;
				continue;
			}

			void* m_pfnHandle = ExtModule->GetProc(Command[2]);
			if (m_pfnHandle == NULL) {
				std::cout << "[server]: failed to find function: '" << Command[2] << "'" << std::endl;
				continue;
			}

			std::cout << "[server]: function '" << Command[2] << "' addr = " << std::hex << "0x" << (uint32_t)m_pfnHandle << std::dec << std::endl;
		}
		else if (Command[0] == "CmdToggleLogging") {
			m_bEnableLogging = !m_bEnableLogging;
		}
		else if (Command[0] == "CmdLoadPlugin") {
			if (Command.size() < 3) {
				std::cout << "CmdLoadPlugin <Name:String> <Path:String>" << std::endl;
				continue;
			}
			XPlatform::core::XPlatformExtensionInfo PluginExtInfo{};
			PluginExtInfo.s_Name = Command[1];
			PluginExtInfo.s_Path = Command[2];
			PluginExtInfo.ExtId = m_pEngine->GetExtensionsInfoList().size();
			if (m_pEngine->LoadExtension(PluginExtInfo) != XPlatform::Api::XPResult::XPLATFORM_RESULT_SUCCESS)std::cout << "[server]: failed to load plugin!" << std::endl;
		}
		else if (Command[0] == "CmdUnLoadPlugin") {
			if (Command.size() < 2) {
				std::cout << "CmdUnLoadPlugin <Name:String>" << std::endl;
				continue;
			}
			
			if (Command[1] == XPLATFORM_NET_EXT_NAME) {
				std::cout << "[server]cannot disable xplatform network extension! :" << Command[1] << std::endl;
				continue;
			}

			//if (m_pEngine->UnLoadExtension(Command[1]) != XPlatform::Api::XPResult::XPLATFORM_RESULT_SUCCESS) {
			//	std::cout << "[server]failed to unload extension: " << Command[1] << std::endl;
			//}

		}
	}
	
	m_ListenThread.join();

	for (std::thread& thrd : m_vThreadPool){
		thrd.join();
	}

	std::cout << "[server] all threads joined!" << std::endl;

	m_pxiSocket->Close();
	m_pxiSocket->Release();

	if (m_pCtx != NULL)m_pCtx->Release();

	std::cout << "[server] all XPlatform objects released!" << std::endl;
	return 0;
}
