#pragma once

#include "structs.hpp"

#define SELECT(mp, zm) (game::environment::t6mp() ? mp : zm)

namespace game
{
	enum gamemode
	{
		none,
		multiplayer,
		zombies
	};

	extern gamemode current;

	namespace environment
	{
		bool t6mp();
		bool t6zm();
	}

	template <typename T>
	class symbol
	{
	public:
		symbol(const size_t t6mp, const size_t t6zm)
			: t6mp_(reinterpret_cast<T*>(t6mp))
			, t6zm_(reinterpret_cast<T*>(t6zm))
		{
		}

		T* get() const
		{
			if (environment::t6mp())
			{
				return t6mp_;
			}

			return t6zm_;
		}

		operator T* () const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}

	private:
		T* t6mp_;
		T* t6zm_;
	};

	scr_entref_t Scr_GetEntityIdRef(unsigned int entId);
}

#include "symbols.hpp"