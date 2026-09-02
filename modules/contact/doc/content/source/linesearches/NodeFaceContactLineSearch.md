# NodeFaceContactLineSearch

This class (derived from [ContactLineSearch.md]) implements a custom line search (based
on the Petsc `LineSearchShell`) for use with node-face mechanical contact. The line
search is not fancy. It takes two parameters, set in the MOOSE Executioner block:
`contact_line_search_ltol` and `contact_line_search_allowed_lambda_cuts`. The
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

This line search helps significantly with the phenomenon in the contact
literature known as jamming or zig-zagging [!citep](wriggers2006computational)
where a Newton solve bounces back and forth between
different contact sets. However, it is not a panacea. It will only partially
assuage problems associated with "bad" linear solves which can arise during
Jacobian-Free Newton-Krylov solves with noisy functions. Function noise can be
introduced through large penalty factors and/or poor scaling of variables.

!bibtex bibliography
