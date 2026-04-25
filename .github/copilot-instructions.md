# Copilot Instructions For pyifb

This repository contains a CPython C extension (`pyifb/ifb.c`) that wraps Fortran CFI APIs. Follow these rules for all edits.

## C API Ownership Rules

- Any `PyObject*` return from getters/methods must be a new reference.
- Use `Py_RETURN_NONE`, `Py_RETURN_TRUE`, `Py_RETURN_FALSE`, or `PyBool_FromLong` for singletons/bools.
- Never return borrowed references from extension functions.

## Module Init And Error Semantics

- `PyMODINIT_FUNC` must return `NULL` on failure, never `Py_None`.
- Preserve Python exceptions on failure paths.

## Type Registration And References

- `PyModule_AddObjectRef` does not steal references.
- After successful `PyModule_AddObjectRef`, `Py_DECREF` the local reference.
- On `PyModule_AddObjectRef` failure, decref owned locals before returning.

## Constructor/Allocator Safety

- In `tp_new`, fetch allocator from the concrete subtype (`PyType_GetSlot(subtype, Py_tp_alloc)`).
- Validate slot pointers before calling through them.
- Return object pointers as `PyObject*` from constructors.
- In `tp_dealloc`, fetch `Py_tp_free` via `PyType_GetSlot(Py_TYPE(self), Py_tp_free)` and call through that function pointer.
- Under `Py_LIMITED_API`, do not access `PyTypeObject` fields directly (e.g., `tp->tp_free`).

## Mutable State Rollback Pattern

- If descriptor fields are mutated before operations that can fail, snapshot original values first.
- Route all post-mutation failures through a shared rollback path.
- Keep object state unchanged on Python-side conversion or validation failures.
- Include `elem_len`, type, version, attribute, and ownership fields in rollback snapshots when they can change.
- Prefer a single `error_restore:` label to ensure both memory cleanup and state rollback happen on every error path.

## Ownership Rebind Safety

- For rebinding operations (`establish`, `section`, `select_part`, `setpointer`), reject calls when the destination descriptor currently owns allocated memory unless that memory is explicitly deallocated first.
- Prefer explicit `ValueError` guards over implicit frees to avoid silent ownership changes and hard-to-debug leaks.

## Sub-Interpreter Safety

- The module declares `Py_MOD_PER_INTERPRETER_GIL_SUPPORTED` and `Py_MOD_GIL_NOT_USED`; all changes must preserve this.
- Do not introduce static or global `PyObject*` variables. All Python object state must live in `IFBModuleState` (stored via `m_size`/`PyModule_GetState`).
- New heap types whose instances need to reference module-owned objects (e.g. `CFI_dim_t`) must be created with `PyType_FromModuleAndSpec` so that `PyType_GetModule` works on their instances.
- Store per-interpreter type references in `IFBModuleState` and retrieve them via `PyType_GetModule(Py_TYPE(self))` + `PyModule_GetState`.
- `IFBModule_traverse` and `IFBModule_clear` must visit/clear every `PyObject*` stored in `IFBModuleState`; update both when adding new state fields.
- Never call `PyType_FromSpec` at call time to manufacture a throw-away type; always reuse the type stored in module state.

## Python Import And Typing Hygiene

- Keep imports clean: avoid unused imports, avoid wildcard imports, and keep implementation-only imports private (prefer underscore-prefixed aliases in `__init__.py`).
- Keep module exports explicit using `__all__` and do not leak unrelated helper symbols into the public package namespace.
- Add and preserve type annotations for public Python APIs and changed code paths; prefer precise types over `Any` when practical.
- Keep `.pyi` stubs in sync with runtime modules when public APIs change.
- Validate Python typing changes with `mypy` before finishing.

## Required Validation

- Run focused tests after C extension changes:
  - `python setup.py build_ext --inplace`
  - `python -m pytest tests/test_cfidesc_wrapper.py`
  - `python -m pytest tests/test_fortran.py`
  - `python -m pytest tests/test_cdesc.py`
- If tests cannot run, report why in the PR/summary.
