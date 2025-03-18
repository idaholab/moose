# StaticCondensation

The `StaticCondensation` preconditioner can be used to condense element interior degrees of freedom out of the global system. For instance, if using a second order Lagrange basis on a `QUAD9` element, the degrees of freedom associated with the element center nodes may be condensed out. Of more practical interest is statically condensing [Hybridizable Discontinuous Galerkin (HDG)](HDGKernels/index.md) discretizations, removing element interior degrees of freedom and leaving only the facet degrees of freedom. In some applications, it may be important to retain some element interior degress of freedom, for instance pressure degrees of freedom in a hybridizable local discontinuous Galerkin (LDG-H) of the Navier-Stokes equations. This may be done using the [!param](/Preconditioning/StaticCondensation/dont_condense_vars) parameter.

!syntax parameters /Preconditioning/StaticCondensation

!syntax inputs /Preconditioning/StaticCondensation

!syntax children /Preconditioning/StaticCondensation




