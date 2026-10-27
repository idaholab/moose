# The PorousFlowDictator

The [PorousFlowDictator](PorousFlowDictator.md) is a `UserObject`
that holds information about the nonlinear variables used in the PorousFlow module,
as well as the number of fluid phases and fluid components in each simulation.

Other PorousFlow objects, such as `Kernels` or `Materials` query the PorousFlowDictator
to make sure that only valid fluid components or phases are used.

The PorousFlowDictator also resolves the FE type of the variables that nodal `Materials` read at
the nodes, and produces an error if they do not all share one.  See
[which nodes are evaluated](mass_lumping.md#which_nodes) for why they must.

!alert note
A PorousFlowDictator must be present in all simulations!
