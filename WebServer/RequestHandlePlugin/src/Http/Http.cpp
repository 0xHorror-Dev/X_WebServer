#include "Http.h"

#include <string>
#include <stdint.h>
#include <sstream>

void XPlatform::Net::HttpRequest::Parse(const std::string& Req) {
	std::string TmpBuf;
	std::ptrdiff_t i = 0;
	// Method:
	for (; i < Req.length(); i++) {
		if (Req[i] == ' ') {
			if (TmpBuf == "GET") m_Type = XPlatform::Net::HTTP_REQUEST_TYPE::HTTP_REQUEST_TYPE_GET;
			else if (TmpBuf == "POST") m_Type = XPlatform::Net::HTTP_REQUEST_TYPE::HTTP_REQUEST_TYPE_POST;
			i++;
			break;
		}
		TmpBuf.push_back(Req[i]);
	}

	TmpBuf.clear();

	// ref
	for (; i < Req.length(); i++) {
		if (Req[i] == ' ') {
			if (TmpBuf == "/") m_sRef = "/index.html";
			else m_sRef = TmpBuf;

			break;
		}
		TmpBuf.push_back(Req[i]);
	}
	i++;

	TmpBuf.clear();

	// version
	for (; i < Req.length(); i++) {
		if (Req[i] == '\n' || Req[i] == '\r') {
			if (TmpBuf == "HTTP/1.1") {
				m_nHttpVersion = 11;
			}
			if (TmpBuf == "HTTP/2.0") {
				m_nHttpVersion = 20;
			}
			break;
		}
		TmpBuf.push_back(Req[i]);
	}

	TmpBuf.clear();
}

std::string HttpVersionToString(uint32_t HttpVersion) {
	if (HttpVersion == 11) return "HTTP/1.1";
	else if (HttpVersion == 20) return "HTTP/2.0";
}

std::string XPlatform::Net::HttpResponse::Prase() {
	std::stringstream ss;

	ss << HttpVersionToString(m_nHttpVersion) << " " << std::to_string(m_nCode) << " " << m_sCodeRew << "\r\n"
		<< "Content-Length: " << m_sContent.length() << "\r\n";
		
	if (!m_sContent_Type.empty())
		ss << "Content-Type: " << m_sContent_Type;
		
		ss << "charset=utf-8" << "\r\n"
		<< "Connection: " << "close"
		<< "\r\n\r\n"
		<< m_sContent;

	return ss.str();
}
