#include "PCH.h"

#include <Windows.h>

#undef GetComputerName

namespace F4SE
{
	namespace WinAPI
	{
		bool(GetComputerName)(
			char* a_computerName,
			std::uint32_t* a_size) noexcept
		{
			return static_cast<bool>(
				::GetComputerNameA(
					static_cast<::LPSTR>(a_computerName),
					reinterpret_cast<::LPDWORD>(a_size)));
		}
	}
}

#ifdef F4SE_SUPPORT_XBYAK
#include <xbyak/xbyak.h>

namespace stl
{
	namespace detail
	{
		struct asm_patch :
			Xbyak::CodeGenerator
		{
			asm_patch(std::uintptr_t a_dst)
			{
				Xbyak::Label dst;

				jmp(ptr[rip + dst]);

				L(dst);
				dq(a_dst);
			}
		};
	}

	void asm_jump(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to)
	{
		detail::asm_patch p{ a_to };
		p.ready();
		assert(p.getSize() <= a_size);
		REL::safe_write(
			a_from,
			std::span{ p.getCode<const std::byte*>(), p.getSize() });
	}

	void asm_replace(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to)
	{
		REL::safe_fill(a_from, REL::INT3, a_size);
		asm_jump(a_from, a_size, a_to);
	}
}
#endif
