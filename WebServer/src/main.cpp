#include <XPlatform.core/loader.h>
#include <XPlatform.core/engine.h>

#include<iostream>
#include<sstream>

#include<cassert>

#include "WebServerApplication/WebServerApplication.h"

#include <fstream>

int main(int argc, char* argv[]) {
	XPlatform::core::XPlatfromInit();

	XPlatform::core::Engine* p_Engine = XPlatform::core::Engine::GetInstance();
	XPlatform::Api::XPResult Res = p_Engine->LoadEngine("project.json");
	assert(Res == XPlatform::Api::XPResult::XPLATFORM_RESULT_SUCCESS);


	XPlatform::XP_IApplication* m_iApp = new XPlatform::Net::WebServerApplication;
	int res = m_iApp->Init(argc, argv);
	std::cout << "[main] initialization result = " << res << std::endl;
	if (res < 0) {
		delete reinterpret_cast<XPlatform::Net::WebServerApplication*>(m_iApp);
		return -1;
	}
	
	res = m_iApp->Run();

	delete reinterpret_cast<XPlatform::Net::WebServerApplication*>(m_iApp);

	std::cout << "[server] server application successfuly stopped with code:" << res << std::endl;

	XPlatform::core::XPlatfromShutdown();
	return 0;
}
