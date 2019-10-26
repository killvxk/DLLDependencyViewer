#pragma once


#include "dbghelp.h"
#include "pe.h"
#include "thread_worker.h"
#include "unique_strings.h"

#include <string>
#include <vector>


struct symbols_from_addresses_param_t
{
	wstring const* m_module_path;
	pe_export_table_info* m_eti;
	std::vector<std::uint16_t> m_indexes;
	std::vector<std::string> m_strings;
};

struct undecorated_from_decorated_param_t
{
	pe_export_table_info* m_eti;
	std::vector<std::uint16_t> m_indexes;
	std::vector<std::string> m_strings;
};


class dbg_provider
{
public:
	static void init();
	static void deinit();
	static dbg_provider* get();
private:
	dbg_provider();
	~dbg_provider();
public:
	void add_task(thread_worker_function_t const func, thread_worker_param_t const param);
	void get_symbols_from_addresses_task(symbols_from_addresses_param_t& param);
	void get_undecorated_from_decorated_task(undecorated_from_decorated_param_t& param);
private:
	void init_task();
	void deinit_task();
private:
	bool m_sym_inited;
	dbghelp m_dbghelp;
	thread_worker m_thread_worker;
};
