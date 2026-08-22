# ContactLineSearch

This class implements the infrastructure shared by the contact-aware line searches
([NodeFaceContactLineSearch.md]): creation and
configuration of a secondary, standalone PETSc `SNESLineSearch` of the type given by the
`backing_line_search` parameter (`basic`, `bt`, `l2`, or `cp`), under the
`contact_backing_` PETSc options prefix, and the optional linear-tolerance loosening
applied while the contact set is changing.

When the contact set is changing, the user may optionally use a looser linear tolerance set by
the `contact_ltol` parameter. Then when the contact set is changing during the
beginning of the Newton solve, unnecessary computational expense is avoided. Then when the
contact set is resolved late in the Newton solve, the linear tolerance will return to the finer
tolerance set through the traditional `l_tol` parameter.

If a developer wants to implement the line-search with a solver other than Petsc
they will have to create the class themself.
