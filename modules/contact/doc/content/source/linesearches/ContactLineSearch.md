# ContactLineSearch

This class (through a `PetscContactLineSearch` derivation) implements a custom
line search (based on the Petsc `LineSearchShell`) for use with mechanical
contact. The line search is not fancy. It takes two parameters, set in the MOOSE
Executioner block: `contact_line_search_ltol` and
`contact_line_search_allowed_lambda_cuts`. The
`contact_line_search_allowed_lambda_cuts` parameter specifies the number of
times the line search is allowed to cut lambda. If allowed to be cut, lambda
will be reduced by half, and a new residual will be evaluated. If the residual
is smaller with a smaller lambda, then cuts will continue until reaching
`contact_line_search_allowed_lambda_cuts`. If the residual is larger with a
smaller lambda, then the line search is curtailed and the smaller residual is
used. It's recommended that `contact_line_search_allowed_lambda_cuts` be <= 3,
with smaller values being used for smaller contact problems. This is to allow
necessary residual increases when the transient problem requires significant
changes in the contact state.

When the contact set is changing, the user may optionally use a looser linear tolerance set by
the `contact_line_search_ltol` parameter. Then when the contact set is changing during the
beginning of the Newton solve, unnecessary computational expense is avoided. Then when the
contact set is resolved late in the Newton solve, the linear tolerance will return to the finer
tolerance set through the traditional `l_tol` parameter.

This line search helps significantly with the phenomenon in the contact
literature known as jamming or zig-zagging [!citep](wriggers2006computational)
where a Newton solve bounces back and forth between
different contact sets. However, it is not a panacea. It will only partially
assuage problems associated with "bad" linear solves which can arise during
Jacobian-Free Newton-Krylov solves with noisy functions. Function noise can be
introduced through large penalty factors and/or poor scaling of variables.

If a developer wants to implement the line-search with a solver other than Petsc
they will have to create the class themself.

!bibtex bibliography
