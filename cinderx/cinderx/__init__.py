"""High-performance Python runtime extensions."""

import os
import sys

try:
    # We need to make the symbols from _cinderx available process wide as they
    # are used in other CinderX modules like _static, etc.
    old_dlopen_flags: int = sys.getdlopenflags()
    sys.setdlopenflags(old_dlopen_flags | os.RTLD_GLOBAL)
    try:
        from _cinderx import (
            # Methods
            _compile_perf_trampoline_pre_fork,
            _get_entire_call_stack_as_qualnames_with_lineno_and_frame,
            _get_entire_call_stack_as_qualnames_with_lineno,
            _is_compile_perf_trampoline_pre_fork_enabled,
            clear_all_shadow_caches,
            clear_caches,
            clear_classloader_caches,
            clear_type_profiles,
            disable_parallel_gc,
            enable_parallel_gc,
            get_and_clear_type_profiles_with_metadata,
            get_and_clear_type_profiles,
            get_parallel_gc_settings,
            init,
            set_profile_interp_all,
            set_profile_interp_period,
            set_profile_interp,
            strict_module_patch_delete,
            strict_module_patch_enabled,
            strict_module_patch,
            watch_sys_modules,

            # Other attributes
            async_cached_classproperty,
            async_cached_property,
            cached_classproperty,
            cached_property,
            StrictModule,
        )
    finally:
        sys.setdlopenflags(old_dlopen_flags)
except ImportError:
    pass
