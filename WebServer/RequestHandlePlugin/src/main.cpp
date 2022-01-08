#include <XPlatform.core/api.h>
#define _PLUGIN_API extern "C" XPLATFORM_API_EXPORT

#include <XPlatform.net/XP_ISocket.h>

#include<string>
#include<fstream>

#include "Http/Http.h"

_PLUGIN_API XPlatform::Api::XPResult xplatform_extension_init() {
	return XPlatform::Api::XPResult::XPLATFORM_RESULT_SUCCESS;
}

_PLUGIN_API void HandleRequest(const std::string& Request, const std::string& m_srSiteFolder, XPlatform::Net::XP_ISocket* m_pxiClient) {
	XPlatform::Net::HttpRequest Req;

	std::ifstream FileStream;
	if (!Request.empty()) {
		Req.Parse(Request);
		
		FileStream.open(m_srSiteFolder + Req.m_sRef, std::ios::binary);

	}
	else if (!FileStream.is_open()) {
		XPlatform::Net::HttpResponse Resp;
		Resp.m_nCode = 404;
		Resp.m_nHttpVersion = !Request.empty() ? Req.m_nHttpVersion : 11;
		Resp.m_sCodeRew = "Not Found";
		Resp.m_sContent = "<h1>File not found!</h1>\n<h2>404</h2>";
		Resp.m_sContent_Type = "text/html";
		std::string _r = Resp.Prase();
		m_pxiClient->Send(_r.c_str(), _r.length());
		return;
	}

	XPlatform::Net::HttpResponse Resp;
	Resp.m_nCode = 200;
	Resp.m_nHttpVersion = Req.m_nHttpVersion;
	Resp.m_sCodeRew = "OK";
	Resp.m_sContent = std::string((std::istreambuf_iterator<char>(FileStream)), std::istreambuf_iterator<char>());
	//Resp.m_sContent_Type = "text/html";
	std::string _r = Resp.Prase();
	m_pxiClient->Send(_r.c_str(), _r.length());
}

_PLUGIN_API void xplatform_extension_shutdown() {

}