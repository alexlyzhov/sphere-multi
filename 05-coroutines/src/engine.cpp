#include <coroutine/engine.h>

#include <iostream>
#include <string.h>
#include <setjmp.h>

namespace Coroutine {

	/**
	 * Save stack of the current coroutine in the given context
	 */

	void Engine::Store(context& ctx) {
		ctx.Low = StackBottom;
		char tmp; // exists only to capture current address
		ctx.High = &tmp;

		if ((char *) ctx.High <= ctx.Low) {
			char *tmp = ctx.Low;
			ctx.Low = ctx.High;
			ctx.High = tmp;
		}

		uint32_t size = ctx.High - ctx.Low;
		
		char *buf = std::get<0>(ctx.Stack); // check existing context properties

		if (std::get<1>(ctx.Stack) < size || buf == nullptr) {
			if (buf != nullptr) {
				delete[] buf;
			}
			// allocate memory for the stack anew
			buf = new char[size];
		}

		// store the stack
		memcpy(buf, ctx.Low, size); // store stack data in buf

		std::get<0>(ctx.Stack) = buf;
		std::get<1>(ctx.Stack) = size;
	}

	/**
	 * Restore stack of the given context and pass control to coroutine
	 */

	void Engine::Restore(context& ctx) {
	    char tmp;

	    if (&tmp >= ctx.Low - std::get<1>(ctx.Stack)) {
			Restore(ctx); // not enough memory yet
		}

		memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
		longjmp(ctx.Environment, 1);
	}

    /**
     * Gives up current routine execution and let engine to schedule other one. It is not defined when
     * routine will get execution back, for example if there are no other coroutines then execution could
     * be trasferred back immediately (yieled turns to be noop).
     *
     * Also there are no guarantee what coroutine will get execution, it could be caller of the current one or
     * any other which is ready to run
     */
	void Engine::yield() {
		context *head = alive;
		// rewrite all these loops. maybe for here?
		while (head) { // if no other coroutines, return immediately
			if (head == cur_routine) {
				head = head->next; // skip current
				continue;
			}

			sched(head);
		}
	}

    /**
     * Suspend current routine and transfers control to the given one, resumes its execution from the point
     * when it has been suspended previously.
     *
     * If routine to pass execution to is not specified runtime will try to transfer execution back to caller
     * of the current routine, if there is no caller then this method has same semantics as yield
     */
	void Engine::sched(void *sched_routine) {
		if (sched_routine == nullptr) {
			if (cur_routine != nullptr) { // cur_routine does not exist until the first sched() call
				if (cur_routine->caller != nullptr) {
					sched(cur_routine->caller); // according to the definition
				} else {
					yield();
				}
			} else {
				longjmp(idle.Environment, 1);
			}
		} else {
			context *sched_ctx = (context *) sched_routine; // new context to schedule
			
			if (cur_routine != nullptr) {  // cur_routine does not exist until the first sched() call
				if (setjmp(cur_routine->Environment) == 0) { // save environment
					Store(*cur_routine); // save stack
				
					if (cur_routine->caller == sched_ctx) { // to avoid calling cycles
						cur_routine->caller = nullptr;
					} else {
						sched_ctx->caller = cur_routine;
					}
					
					cur_routine->callee = sched_ctx;
				} else {
					return;
				}
			} 

			cur_routine = sched_ctx;
			Restore(*sched_ctx);
		}
	}

} // namespace Coroutine
