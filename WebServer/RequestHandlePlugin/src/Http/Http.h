#ifndef _XPLATFORM_HTTP_H
#define _XPLATFORM_HTTP_H

#include<string>
#include<stdint.h>

namespace XPlatform {
	namespace Net {
		enum HTTP_REQUEST_TYPE {
			HTTP_REQUEST_TYPE_UNKNOWN,
			HTTP_REQUEST_TYPE_GET,
			HTTP_REQUEST_TYPE_POST
		};

		struct HttpRequest {
			HTTP_REQUEST_TYPE m_Type;
			std::string m_sRef;
			uint32_t m_nHttpVersion;

			void Parse(const std::string& Req);
		};

		struct HttpResponse {
			uint32_t m_nCode;
			uint32_t m_nHttpVersion;
			std::string m_sContent_Type;
			std::string m_sCodeRew;
			std::string m_sContent;

			std::string Prase();
		};
	}
}

#endif