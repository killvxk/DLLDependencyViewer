#include "dbghelp.h"

#include "smart_reg_key.h"

#include <array>
#include <cassert>
#include <filesystem>


dbghelp::dbghelp():
	m_dbghelp_dll(),
	m_fn_SymGetOptions(),
	m_fn_SymSetOptions(),
	m_fn_SymInitializeW(),
	m_fn_SymCleanup(),
	m_fn_SymLoadModuleExW(),
	m_fn_SymUnloadModule64(),
	m_fn_SymFromAddr(),
	m_fn_UnDecorateSymbolName()
{
}

dbghelp::~dbghelp()
{
}

bool dbghelp::init()
{
	load_dlls();
	if(!m_dbghelp_dll)
	{
		return false;
	}
	m_fn_SymGetOptions       	= reinterpret_cast<decltype(&SymGetOptions       	)>(get_function_address(m_dbghelp_dll, "SymGetOptions"       	)); if(!m_fn_SymGetOptions       	) { m_dbghelp_dll.reset(); return false; }
	m_fn_SymSetOptions       	= reinterpret_cast<decltype(&SymSetOptions       	)>(get_function_address(m_dbghelp_dll, "SymSetOptions"       	)); if(!m_fn_SymSetOptions       	) { m_dbghelp_dll.reset(); return false; }
	m_fn_SymInitializeW      	= reinterpret_cast<decltype(&SymInitializeW      	)>(get_function_address(m_dbghelp_dll, "SymInitializeW"      	)); if(!m_fn_SymInitializeW      	) { m_dbghelp_dll.reset(); return false; }
	m_fn_SymCleanup          	= reinterpret_cast<decltype(&SymCleanup          	)>(get_function_address(m_dbghelp_dll, "SymCleanup"          	)); if(!m_fn_SymCleanup          	) { m_dbghelp_dll.reset(); return false; }
	m_fn_SymLoadModuleExW    	= reinterpret_cast<decltype(&SymLoadModuleExW    	)>(get_function_address(m_dbghelp_dll, "SymLoadModuleExW"    	)); if(!m_fn_SymLoadModuleExW    	) { m_dbghelp_dll.reset(); return false; }
	m_fn_SymUnloadModule64   	= reinterpret_cast<decltype(&SymUnloadModule64   	)>(get_function_address(m_dbghelp_dll, "SymUnloadModule64"   	)); if(!m_fn_SymUnloadModule64   	) { m_dbghelp_dll.reset(); return false; }
	m_fn_SymFromAddr         	= reinterpret_cast<decltype(&SymFromAddr         	)>(get_function_address(m_dbghelp_dll, "SymFromAddr"         	)); if(!m_fn_SymFromAddr         	) { m_dbghelp_dll.reset(); return false; }
	m_fn_UnDecorateSymbolName	= reinterpret_cast<decltype(&UnDecorateSymbolName	)>(get_function_address(m_dbghelp_dll, "UnDecorateSymbolName"	)); if(!m_fn_UnDecorateSymbolName	) { m_dbghelp_dll.reset(); return false; }
	return true;
}

void dbghelp::load_dlls()
{
	#if defined _M_IX86
	static constexpr wchar_t const s_windows_kits_installed_roots[] = LR"---(SOFTWARE\Wow6432Node\Microsoft\Windows Kits\Installed Roots)---";
	static constexpr wchar_t const s_debuggers_sub_directory[] = LR"---(debuggers\x86)---";
	#elif defined _M_X64
	static constexpr wchar_t const s_windows_kits_installed_roots[] = LR"---(SOFTWARE\Microsoft\Windows Kits\Installed Roots)---";
	static constexpr wchar_t const s_debuggers_sub_directory[] = LR"---(debuggers\x64)---";
	#else
	#error Unknown architecture.
	#endif
	static constexpr wchar_t const s_windows_10_sdk[] = L"KitsRoot10";
	static constexpr wchar_t const s_windows_8_1_sdk[] = L"KitsRoot81";
	static constexpr wchar_t const s_windows_8_sdk[] = L"KitsRoot";
	static constexpr wchar_t const s_dbghelp_file_name[] = L"dbghelp.dll";

	HKEY installed_roots;
	LSTATUS const opened = RegOpenKeyExW(HKEY_LOCAL_MACHINE, s_windows_kits_installed_roots, 0, KEY_QUERY_VALUE, &installed_roots);
	if(opened != ERROR_SUCCESS)
	{
		m_dbghelp_dll = load_library(s_dbghelp_file_name);
		return;
	}
	auto const sp_installed_roots = make_smart_reg_key(installed_roots);
	static constexpr wchar_t const* const s_values_to_try[] = {s_windows_10_sdk, s_windows_8_1_sdk, s_windows_8_sdk};
	auto const buff = std::make_unique<std::array<wchar_t, 32 * 1024>>();
	DWORD size;
	for(auto const& value_name : s_values_to_try)
	{
		DWORD type;
		size = static_cast<DWORD>(buff->size() * sizeof(wchar_t));
		LSTATUS const read =  RegGetValueW(installed_roots, L"", value_name, RRF_RT_REG_SZ, &type, buff->data(), &size);
		if(read != ERROR_SUCCESS)
		{
			continue;
		}
		assert(size <= buff->size() * sizeof(wchar_t));
		assert(size >= 2 * sizeof(wchar_t));
		assert(size % sizeof(wchar_t) == 0);
		assert((*buff)[(size / sizeof(wchar_t)) - 1] == L'\0');
		auto p = std::filesystem::path(buff->data(), buff->data() + (size / sizeof(wchar_t)) - 1).append(s_debuggers_sub_directory).append(s_dbghelp_file_name);
		m_dbghelp_dll = load_library(p.c_str());
		if(!m_dbghelp_dll)
		{
			continue;
		}
		return;
	}
	m_dbghelp_dll = load_library(s_dbghelp_file_name);
}
