#include <stdinc.hpp>
#include "execution.hpp"
#include "safe_execution.hpp"
#include "stack_isolation.hpp"

#include "component/scripting.hpp"

namespace scripting
{
	namespace
	{
		game::VariableValue* allocate_argument()
		{
			game::VariableValue* value_ptr = ++game::scr_VmPub->top;
			++game::scr_VmPub->inparamcount;
			return value_ptr;
		}

		void push_value(const script_value& value)
		{
			auto* value_ptr = allocate_argument();
			*value_ptr = value.get_raw();
		}

		script_value get_return_value()
		{
			if (game::scr_VmPub->inparamcount == 0)
			{
				return {};
			}

			game::Scr_ClearOutParams(game::SCRIPTINSTANCE_SERVER);
			game::scr_VmPub->outparamcount = game::scr_VmPub->inparamcount;
			game::scr_VmPub->inparamcount = 0;

			return script_value(game::scr_VmPub->top[1 - game::scr_VmPub->outparamcount]);
		}

		int get_field_id(const int classnum, const std::string& field)
		{
			if (scripting::fields_table.find(field) != scripting::fields_table.end())
			{
				return scripting::fields_table[field];
			}

			return -1;
		}
	}

	void notify(const entity& entity, const std::string& event, const std::vector<script_value>& arguments)
	{
		for (auto i = arguments.rbegin(); i != arguments.rend(); ++i)
		{
			push_value(*i);
		}

		const auto event_id = game::SL_GetString(event.data(), 0);
		game::Scr_NotifyId(game::SCRIPTINSTANCE_SERVER, 0, entity.get_entity_id(), event_id, game::scr_VmPub->inparamcount);
	}

	script_value call_function(const std::string& name, const entity& entity,
		const std::vector<script_value>& arguments)
	{
		const auto entref = entity.get_entity_reference();

		game::Scr_ClearOutParams(game::SCRIPTINSTANCE_SERVER);

		for (auto i = arguments.rbegin(); i != arguments.rend(); ++i)
		{
			push_value(*i);
		}

		game::scr_VmPub->outparamcount = game::scr_VmPub->inparamcount;
		game::scr_VmPub->inparamcount = 0;

		const auto func = scripting::find_function(name);

		if (!func)
		{
			throw std::runtime_error("Unknown function '" + name + "'");
		}

		func(entref);

		return get_return_value();
	}

	script_value call_function(const std::string& name, const std::vector<script_value>& arguments)
	{
		return call_function(name, entity(), arguments);
	}

	template <>
	script_value call(const std::string& name, const std::vector<script_value>& arguments)
	{
		return call_function(name, arguments);
	}

	void set_entity_field(const entity& entity, const std::string& field, const script_value& value)
	{
		const auto entref = entity.get_entity_reference();

		const int id = get_field_id(entref.classnum, field);

		if (id != -1)
		{
			push_value(value);

			game::scr_VmPub->outparamcount = game::scr_VmPub->inparamcount;
			game::scr_VmPub->inparamcount = 0;

			game::Scr_SetObjectField(entref.classnum, entref.entnum, id);
		}
	}

	script_value get_entity_field(const entity& entity, const std::string& field)
	{
		const auto entref = entity.get_entity_reference();

		const int id = get_field_id(entref.classnum, field);

		if (id != -1)
		{
			const auto value = game::GetEntityFieldValue(game::SCRIPTINSTANCE_SERVER, entref.classnum, entref.entnum, 0, id);

			return value;
		}

		return {};
	}
}