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

		uint32_t size = forward ? (ctx.High - ctx.Low) : (ctx.Low - ctx.High);
		
		char *buf = std::get<0>(ctx.Stack); // check existing context properties

		if (std::get<1>(ctx.Stack) < size || buf == nullptr) {
			if (buf) {
				delete [] buf;
			}
			// allocate memory for the stack anew
			buf = new char[size];
		}

		// store the stack
		memcpy(buf, forward ? ctx.Low : ctx.High, size);

		std::get<0>(ctx.Stack) = buf;
		std::get<1>(ctx.Stack) = size;
	}

	/**
	 * Restore stack of the given context and pass control to coroutine
	 */

	void Engine::Restore(context& ctx) {
	    char tmp;
	    if ((forward && &tmp <= ctx.High) || (!forward && &tmp >= ctx.High - std::get<1>(ctx.Stack))) {
			Restore(ctx); // not enough memory yet
		}

		memcpy(forward ? ctx.Low : ctx.High, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
		longjmp(ctx.Environment, 1);
	}

    /**
     * Gives up current routine execution and let engine to schedule other one. It is not defined when
     * routine will get execution back, for example if there are no other coroutineutines then execution could
     * be trasferred back immediately (yieled turns to be noop).
     *
     * Also there are no guarantee what coroutine will get execution, it could be caller of the current one or
     * any other which is ready to run
     */
	void Engine::yield() {
	    if (alive == nullptr) {
	    	// if no other coroutines, then transfer control back
	    	return;
	    }
		context *head = alive;
		while (head) { // rewrite all these loops. maybe for here?
			if (head == cur_routine) {
				head = head->next; // skip current
				continue;
			}
			context *check = head; // ensure that it's non-parent
			while (check->callee) {
				if (check->callee == cur_routine) {
					head = head->next;
					break;
				}
			}

			if (!check->callee) {
				sched(head); // head is an alive & not-parent-of-cur context => sched
			} else {
				continue;
			}
		}
	}

    /**
     * Suspend current routine and transfers control to the given one, resumes its execution from the point
     * when it has been suspended previously.
     *
     * If routine to pass execution to is not specified runtime will try to transfer execution back to caller
     * of the current routine, if there is no caller then this method has same semantics as yield
     */
	void Engine::sched(void *routine) {
		if (routine == nullptr) {
			// try give to caller
			if (cur_routine != nullptr) { // cur_routine does not exist until the first sched() call
				if (cur_routine->caller != nullptr) {
					routine = cur_routine->caller;
					// sched(cur_routine->caller); // according to the definition
				} else {
					yield();
				}
			}
			else {
				longjmp(idle_ctx->Environment, 1);
			}
			return;
		}
		
		context *ctx = (context *) routine;
		
		if (cur_routine != nullptr) {  // cur_routine does not exist until the first sched() call
			if (setjmp(cur_routine->Environment) == 0) { // save environment
				Store(*cur_routine); // save stack
			
				if (cur_routine->caller == ctx) {
					cur_routine->caller = nullptr;
				} else {
					ctx->caller = cur_routine;
				}
				
				cur_routine->callee = ctx;
			} else {
				return;
			}
		} 

		cur_routine = ctx;
		Restore(*ctx);
	}

} // namespace Coroutine
