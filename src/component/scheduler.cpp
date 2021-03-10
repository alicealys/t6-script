#include <stdinc.hpp>

namespace scheduler
{
	namespace
	{
		int spawn_system_update;

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

		void server_frame()
		{
			execute();
			reinterpret_cast<void (*)()>(SELECT(0x43D2F0, 0x481C30))();
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
		utils::hook::call(SELECT(0x4A597E, 0x6AA27E), server_frame);
	}
}