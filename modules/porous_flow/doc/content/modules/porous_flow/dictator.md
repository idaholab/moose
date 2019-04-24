# The PorousFlowDictator

The [PorousFlowDictator](PorousFlowDictator.md) is a `UserObject`
that holds information about the nonlinear variables used in the PorousFlow module,
as well as the number of fluid phases and fluid components in each simulation.

Other PorousFlow objects, such as `Kernels` or `Materials` query the PorousFlowDictator
to make sure that only valid fluid components or phases are used.

!alert note
A PorousFlowDictator must be present in all simulations!
