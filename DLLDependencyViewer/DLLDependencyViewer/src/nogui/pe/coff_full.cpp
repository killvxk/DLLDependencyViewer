#include "coff_full.h"

#include "mz.h"
#include "../assert.h"


bool pe_parse_coff_full_32_64(void const* const& file_data, int const& file_size, coff_full_32_64 const*& hd)
{
	dos_header const& dosheader = *reinterpret_cast<dos_header const*>(file_data);

	coff_header const* coff_hdr;
	e_pe_parse_coff_header const coff = pe_parse_coff_header(file_data, file_size, coff_hdr);
	WARN_M_R(coff == e_pe_parse_coff_header::ok, L"Failed to pe_parse_coff_header.", false);

	coff_optional_header_standard_32_64 const* coff_opt_std;
	e_pe_parse_coff_optional_header_standard_32_64 const coff_optional = pe_parse_coff_optional_header_standard_32_64(file_data, file_size, coff_opt_std);
	WARN_M_R(coff_optional == e_pe_parse_coff_optional_header_standard_32_64::ok, L"Failed to pe_parse_coff_optional_header_standard_32_64.", false);

	coff_optional_header_windows_32_64 const* coff_opt_win;
	e_pe_parse_coff_optional_header_windows_32_64 const coff_windows = pe_parse_coff_optional_header_windows_32_64(file_data, file_size, coff_opt_win);
	WARN_M_R(coff_windows == e_pe_parse_coff_optional_header_windows_32_64::ok, L"Failed to pe_parse_coff_optional_header_windows_32_64.", false);

	bool const is_32 = coff_opt_std->m_32.m_signature == s_coff_optional_sig_32;
	std::uint32_t const& dir_cnt = is_32 ? coff_opt_win->m_32.m_data_directory_count : coff_opt_win->m_64.m_data_directory_count;
	WARN_M_R(dir_cnt <= 16, L"Too many data directories.", false);
	WARN_M_R(file_size >= static_cast<int>(dosheader.m_pe_offset + sizeof(coff_full_32_64)), L"File too small to contain coff_full_32_64.", false);
	hd = reinterpret_cast<coff_full_32_64 const*>(coff_hdr);
	coff_full_32_64 const& header = *hd;
	data_directory const* const directories = is_32 ? header.m_32.m_directories : header.m_64.m_directories;
	WARN_M(dir_cnt < static_cast<std::uint32_t>(e_directory_table::architecture) || (directories[static_cast<int>(e_directory_table::architecture)].m_va == 0 && directories[static_cast<int>(e_directory_table::architecture)].m_size == 0), L"Architecture is reserved, must be 0.");
	WARN_M(dir_cnt < static_cast<std::uint32_t>(e_directory_table::global_ptr) || directories[static_cast<int>(e_directory_table::global_ptr)].m_size == 0, L"The size member of Global Ptr structure must be set to zero.");
	WARN_M(dir_cnt < static_cast<std::uint32_t>(e_directory_table::reserved) || (directories[static_cast<int>(e_directory_table::reserved)].m_va == 0 && directories[static_cast<int>(e_directory_table::reserved)].m_size == 0), L"Reserved, must be zero.");
	return true;
}