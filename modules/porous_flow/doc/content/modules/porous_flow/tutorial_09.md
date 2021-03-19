[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_08.md) |
[Next](porous_flow/tutorial_10.md)

# Porous Flow Tutorial Page 09.  An overview of the PorousFlow architecture

## Introduction

New users will probably find this page is full of gobbledygook, but the information may be useful as they gain experience with PorousFlow.

Sometimes PorousFlow can be very frustrating to run because MOOSE keeps spitting back things like

`Material property 'PorousFlow_matrix_internal_energy_nodal', requested by 'PorousFlowUnsaturated_EnergyTimeDerivative' is not defined on block caps`

You have to create a `Material` that supplies a property called `PorousFlow_matrix_internal_energy_nodal`, but which PorousFlow Material do you choose?  In multi-component, multi-phase problems these error messages get even more obscure.  To debug your input file it is helpful to understand the overall design of PorousFlow.

## The PorousFlow design

Yes, it is actually designed and not just hacked together!  The design is fundamentally influenced by the following requirements

1. Multi-component, multi-phase fluids need to be simulated.  This means that the `Kernels`, `Materials`, etc are all built using C++ `std::vectors` of components and phases, that are sized at runtime when MOOSE reads your input file.  An alternative approach would be to have a separate 1-component, 1-phase module; another module for 2-component, 1-phase; another for 3-component, 1-phase, another for 2-component, 2-phase, etc, as well as all the possible couplings with temperature, mechanics and chemistry.  This would be a huge task, made even worse by the exhaustive (and exhausting) testing and documentation.  Unfortunately, not only do the `std::vectors` make the code quite complex, but they necessitate the Joiners (see below).  However, to make things easier for users, `Actions` have been created that cover the common use-case of single-phase fluid flow.  These have been used in the tutorial so far, but will be abandoned in [Page 10](porous_flow/tutorial_10.md).

2. In many PorousFlow simulations, [mass lumping](porous_flow/mass_lumping.md) and [full upwinding](porous_flow/upwinding.md) or the [Kuzmin-Turek TVD scheme](kt.md) are critical to avoid unphysical results (eg, negative concentrations or saturations) and to vastly improve convergence (to prevent fluid from being withdrawn from a node where there is no fluid).  However, MOOSE is not really suited to this approach because the `Kernels` need all the Material properties (and other things) evaluated at the nodes, rather than at the quadpoints.  A special `PorousFlowMaterial` has been created that allows all other PorousFlow Materials to be evaluated at nodes by specifying the `at_nodes` input parameter. The required version of each material is added automatically, so the user shouldn't need to set the `at_nodes` parameter.

3. In contrast to much of the remainder of MOOSE, good nonlinear convergence is only obtained when the Jacobian is built using all the derivative information (a short discussion has already been presented on [Page 02](porous_flow/tutorial_02.md)).  The `porous_flow_vars` input parameter of the [`PorousFlowDictator`](PorousFlowDictator.md) controls which derivatives are entered into the Jacobian.  However, sometimes the derivatives are extremely complicated (you will see hundreds of lines of chain-rules if you look at the code) so MOOSE's DerivativeMaterial system is used which allows developers to specify the derivatives if they desire, but will return 0 if none are specified.

4. PorousFlow can run the same simulation using a variety of different nonlinear `Variables`!  For instance, a two-phase simulation can use two porepressure variables, or a single porepressure and a saturation variable.  Or a thermally-coupled simulation can use porepressure and temperature, or can use enthalpy as a `Variable` instead.  The reasons for this are: that the choice of `Variables` can greatly affect the nonlinear convergence; and, when phase changes (boiling of a liquid) occur, a new set of variables is needed, or persistant variables that are not just porepressure and temperature, should be used.  PorousFlow provides a set of *fundamental Materials* that compute porepressures, saturations, temperature and mass fractions from the `Variables`.  Gradients of these quantities, and derivatives with the respect to the `Variables` are also computed.  Those porepressures, saturations, temperature and mass fractions are then `Material` properties and are used by everything else in PorousFlow.

## Kernels

Often you're not sure which PorousFlow `Kernel` describes which DE!  The DEs are described in the [governing equations](porous_flow/governing_equations.md) and at the bottom of that page is a table showing the Kernel names and their mathematical expressions.

## Materials

These form the heart of PorousFlow.  Usually the `[Materials]` block in the input file is the longest and most complicated block.

### Nodal materials

Mentioned above is the `at_nodes` input parameter.  The standard in PorousFlow is that a `Material` computes a property called `property_nodal` if `at_nodes = true`, while it computes the property `property_qp` if `at_nodes = false`.

### Derivatives

Almost all PorousFlow `Materials` compute derivatives, and most of them retrieve derivative information from other `Materials` and combine then in ghastly chain-rules.  The standard is that these are called `dproperty_nodal_dvar` and `dproperty_qp_dvar`.  Some `Materials` also compute `dproperty_nodal_dgradvar`, `dgrad_property_qp_dvar`, etc

### Joiners

In the input file, you specify things like densities, relative permeabilities, etc, for each phase or component.  But most PorousFlow `Materials`, `Kernels`, etc, require these to be formatted into C++ `std::vectors`.  This is what the [`PorousFlowJoiner`](PorousFlowJoiner.md) does.  Even single-phase, single-component simulations need Joiners: they just create `std::vectors` with length 1. These are
added automatically by the action system.

### What Material do I need?

The unix `grep` utility is your friend here!  Do the following

```bash
cd porous_flow/src/materials
grep matrix_internal *.C
```

to find the `PorousFlowMatrixInternalEnergy` `Material` needed to solve the above MOOSE "undefined" error.

[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_08.md) |
[Next](porous_flow/tutorial_10.md)
