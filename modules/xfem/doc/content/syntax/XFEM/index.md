# XFEM

## Description

The `XFEM` block must be supplied to run any MOOSE simulations using XFEM. The [XFEMAction](/actions/XFEMAction.md)
is associated with this block and performs the key model setup tasks.  The `XFEM` input syntax block provides an interface
to specify parameters related to XFEM. Including this block in the input file serves several functions:

- Causes an algorithm to be run at defined intervals to split the mesh based on a set of evolving defined interfaces.
- Causes a modified quadrature rule to be used to correctly integrate elements split by these interfaces.
- Creates objects to generate output related to the XFEM algorithm.
- Provides a syntax block where various parameters controlling the behavior of XFEM can be specified by the user, including:

  - Provide `qrule` variable to XFEM to determine which quadrature weighting rule to use.
  - Specify `output_cut_plane` variable to output cut elements to the exodus mesh.
  - Optional: Provide parameter values for prescribed, incremental crack growth.
  - Optional: Setup of near tip enrichment parameters.
  - Optional: Allow users to control the amount of debugging information printed during a simulation.

## Constructed Objects

Three of the above user-settable parameters pass parameters to the XFEM object for use in setting up
relevant options: the `qrule` variable, incremental crack growth, and `debug_output_level`. The
remaining parameters result in the creation of various objects, which allows for input files to be
more concise. The following table details the types of classes created by utilizing the
`output_cut_plane` option in the `XFEMAction`.

!table id=incr_crack_xfem_action_table caption=Correspondence Among Action Functionality and MooseObjects for `XFEMAction` when `output_cut_plane` = `true`.
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Add `AuxVariables` for various cut parameters and volume fraction calculations | [AuxVariables](/AuxVariables/index.md) |   |
| Add `AuxKernels` for various cut parameters and volume fraction calculations | [XFEMVolFracAux](/XFEMVolFracAux.md) [XFEMCutPlaneAux](/XFEMCutPlaneAux.md) |   |

Several objects are instantiated when using the `output_cut_plane` option. These `AuxVariables` and
`AuxKernels` store data defining the XFEM cut: element volume fraction as well as definitions for
two planes (their x, y, z origins and normals). This allows the cut location to be tracked
throughout the simulation and saves the user from having to manually set up each `AuxVariable` and
its associated `AuxKernel` (`XFEMVolFracAux` for the volume fraction and `XFEMCutPlaneAux` for cut
plane AuxVariables).

Next, the class types created when using crack tip enrichment are detailed:

!table id=near_tip_xfem_action_table caption=Correspondence Among Action Functionality and MooseObjects for `XFEMAction` when `use_crack_tip_enrichment` = `true`.
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Add enrichment displacement variables | [Variables](syntax/Variables/index.md) | `enrichment_displacements`: vector of enrichment displacement names |
| Add stress divergence tensor kernels for crack tip enrichment | [CrackTipEnrichmentStressDivergenceTensors](/CrackTipEnrichmentStressDivergenceTensors.md) | `crack_front_definition`, `enrichment_displacements`, `displacements` |
| Add crack tip enrichment boundary conditions | [CrackTipEnrichmentCutOffBC](/CrackTipEnrichmentCutOffBC.md) | `cut_off_boundary`, `cut_off_radius`, `crack_front_definition` |

When crack tip enrichment is used, `enrichment_displacements` are added as first-order, Lagrange
Variables. Next, a `CrackTipEnrichmentStressDivergenceTensors` kernel is added for each enrichment
displacement variable. Each kernel uses the `crack_front_definition`, `enrichment_displacements`,
and `diplacements` provided in the XFEM Action. All enrichment displacement variables also have an
associated `CrackTipEnrichmentCutOffBC` object added by the action from user specified parameters
(`UserObject` `crack_front_definition`, the vector of `BoundaryName` variables `cut_off_boundary`,
and respective `Real` value `cut_off_radius`).

## Example Input File Syntax

Many input files only utilize the `qule` and `output_cut_plane` variables:

!listing modules/xfem/test/tests/bimaterials/glued_bimaterials_2d.i block=XFEM

However, options exist for specifying cracks growing in a prescribed fashion:

!listing modules/xfem/test/tests/solid_mechanics_basic/crack_propagation_2d.i block=XFEM

Additionally, the XFEM Action contains variables used to employ near tip enrichment; of use when a
propagating crack's tip ends within an element:

!listing modules/xfem/test/tests/crack_tip_enrichment/edge_crack_2d.i block=XFEM

Any use of the XFEM action can output varying levels of problem solution detail by changing the
value of the `debug_output_level` variable. By default it is set to 1, but XFEM output can be
either turned off (`debug_output_level=0`) or greater detail can be added to the output by 
setting the variable to 2 or 3.

!syntax parameters /XFEM/XFEMAction

!syntax list /XFEM objects=True actions=False subsystems=False

!syntax list /XFEM objects=False actions=False subsystems=True

!syntax list /XFEM objects=False actions=True subsystems=False
