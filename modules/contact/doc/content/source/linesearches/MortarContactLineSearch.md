# MortarContactLineSearch

This class (derived from [ContactLineSearch.md]) implements a constraint-set-aware line
search for mortar mechanical contact. Rather than implementing its own bisection
algorithm, it wraps a standard PETSc backing `SNESLineSearch` (configured the same way as
[ContactLineSearch.md]'s `backing_line_search` parameter) and only intervenes when the
active contact/stick/slip set changes between the checkpointed iterate and the backing
search's proposed iterate:

- If the set did not change, the backing search's own result is committed unmodified.
- If the set changed but the backing search still achieved a sufficient residual
  reduction (`direct_accept_tol`), its result is committed directly.
- Otherwise, a single step is taken along the Newton direction, bounded by the predicted
  step length at which the first active dof's contact switch value would cross zero.
  Predicted events within `event_group_tol` of each other are grouped and treated as
  simultaneous.
- If none of the above apply, the failure is propagated to the outer SNES unchanged.

Event prediction only tracks each dof's normal (open/closed) switch value; it does not
predict stick/slip transitions for frictional dofs.

The `c`, `normalize_c`, `use_derived_c_normal`, `c_t`, `mu`, and `epsilon` parameters must
be kept consistent with the matching mortar `[Constraints]` block by the user; this class
has no way to automatically cross-check them against the constraints actually assembling
the residual.
