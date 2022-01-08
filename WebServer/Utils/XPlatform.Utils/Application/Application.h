#ifndef _XPLATFORM_APPLICATION_LAYER_INTERFACE_H
#define _XPLATFORM_APPLICATION_LAYER_INTERFACE_H

#include<array>
#include<XPlatform.core/engine.h>

namespace XPlatform {
	class XP_IApplication {
	protected:
		XPlatform::core::Engine* m_pEngine = XPlatform::core::Engine::GetInstance();
	
	public:

		virtual int Init(int argc, char* argv[]) = 0;

		virtual int Run() = 0;


	};
}

#endif