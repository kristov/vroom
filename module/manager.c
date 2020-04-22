#include <duktape.h>
#include "runtime.h"

vrms_module_t* module_glob;

static duk_ret_t native_print(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
    module_glob->interface.debug(module_glob, "%s", duk_safe_to_string(ctx, -1));
	return 0;
}

void* run_module(vrms_module_t* module) {
    module->interface.debug(module, "starting duktape");
	duk_context *ctx = duk_create_heap_default();
    if (!ctx) {
        return NULL;
    }

    module_glob = module;

	duk_push_c_function(ctx, native_print, DUK_VARARGS);
	duk_put_global_string(ctx, "print");
	duk_eval_string(ctx, "print('Hello world!');");
	duk_pop(ctx);

	duk_destroy_heap(ctx);

    return NULL;
}
