#include <stdinc.hpp>

namespace scheduler
{
	namespace
	{
		int glass_update;

		std::queue<std::function<void()>> tasks;

		struct task
		{
			std::function<bool()> handler;
			std::chrono::milliseconds interval{};
			std::chrono::high_resolution_clock::time_point last_call{};
		};

		utils::concurrent_list<task> callbacks;

		void execute()
		{
			for (auto callback : callbacks)
			{
				const auto now = std::chrono::high_resolution_clock::now();
				const auto diff = now - callback->last_call;

				if (diff < callback->interval) continue;

				callback->last_call = now;

				const auto res = callback->handler();
				if (res)
				{
					callbacks.remove(callback);
				}
			}
		}

		__declspec(naked) void server_frame()
		{
			__asm
			{
				pushad
				call execute
				popad

				push glass_update
				retn
			}
		}
	}

	void schedule(const std::function<bool()>& callback, const std::chrono::milliseconds delay)
	{
		task task;
		task.handler = callback;
		task.interval = delay;
		task.last_call = std::chrono::high_resolution_clock::now();

		callbacks.add(task);
	}

	void loop(const std::function<void()>& callback, const std::chrono::milliseconds delay)
	{
		schedule([callback]()
		{
			callback();
			return false;
		}, delay);
	}

	void once(const std::function<void()>& callback, const std::chrono::milliseconds delay)
	{
		schedule([callback]()
		{
			callback();
			return true;
		}, delay);
	}

	void init()
	{
		glass_update = SELECT(0x49E910, 0x5001A0);
		utils::hook::jump(SELECT(0x4A59F7, 0x6AA2F7), server_frame);
	}
}