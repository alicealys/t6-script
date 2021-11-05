#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "notifies.hpp"

namespace notifies
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{

		}
	};
}

REGISTER_COMPONENT(notifies::component)
