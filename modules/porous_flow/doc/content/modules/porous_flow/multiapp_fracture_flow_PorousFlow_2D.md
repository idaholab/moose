# Fracture flow using a MultiApp approach: Porous flow in a single matrix system

## Background

PorousFlow can be used to simulate flow through fractured porous media in the case when the fracture network is so complicated it cannot be incorporated into the porous-media mesh.  The fundamental premise is *the fractures can be considered as lower dimensional entities within the higher-dimensional porous media*.  This page is part of a set that describes a MOOSE MultiApp approach to simulating such models:

- [Introduction](multiapp_fracture_flow_introduction.md)
- [Mathematics and physical interpretation](multiapp_fracture_flow_equations.md)
- [Transfers](multiapp_fracture_flow_transfers.md)
- [MultiApp primer](multiapp_fracture_flow_primer.md): the diffusion equation with no fractures, and quantifying the errors introduced by the MultiApp approach
- [Diffusion in mixed dimensions](multiapp_fracture_flow_diffusion.md)
- [Porous flow in a single matrix system](multiapp_fracture_flow_PorousFlow_2D.md)
- [Porous flow in a small fracture network](multiapp_fracture_flow_PorousFlow_3D.md)

## Porous flow in a single matrix system

Consider a single 1D planar fracture within a 2D mesh.  Fluid flows along the fracture according to Darcy's equation, so may be modelled using [PorousFlowFullySaturated](PorousFlowFullySaturated.md)

!listing single_fracture_heat_transfer/fracture_app.i block=PorousFlowFullySaturated

!alert note
[Kuzmin-Turek](/porous_flow/kt.md) stabilization cannot typically be used for the fracture flow.  This is because multiple fracture elements will meet along a single finite-element edge when fracture planes intersect, while for performance reasons, libMesh assumes only one or two elements share an edge.  This prevents the KT approach from quantifying flows in all neighboring elements, which prevents the scheme from working.  In the case at hand, KT stabilization is possible because there is just a single fracture.

The following assumptions are used (see [Mathematics and physical interpretation](multiapp_fracture_flow_equations.md)).

- The fracture aperture is $10^{-2}\,$m, and its porosity is $1$.  Therefore, the porosity required by PorousFlow is $a\phi = 10^{-2}$.
- The permeability is given by the $a^{2}/12$ [formula](flow_through_fractured_media.md), modified by a roughness coefficient, $r$, so the permeability required by PorousFlow (which includes the extra factor of $a$) is $a^{3}r/12 = 10^{-8}$.
- The fracture is basically filled with water, so the internal energy of any rock material within it may be ignored.
- The thermal conductivity in the fracture is dominated by the water (which has thermal conductivity 0.6$\,$W.m$^{-1}$.K$^{-1}$), so the thermal conductivity required by PorousFlow is $0.6\times 10^{-2}$.

Hence, the Materials are

!listing single_fracture_heat_transfer/fracture_app.i block=Materials

The heat transfer between the fracture and matrix is encoded in the usual way (see the [primer](multiapp_fracture_flow_primer.md) and the [diffusion](multiapp_fracture_flow_diffusion.md) pages).  The heat transfer coefficient is chosen to be 100, just so that an appreciable amount of heat energy is transferred, not because this is a realistic transfer coefficient for this case:

!listing single_fracture_heat_transfer/fracture_app.i block=Kernels

The boundary conditions correspond to injection of 100$^{\circ}$C water at a rate of 10$\,$kg.s$^{-1}$ at the left side of the model, and withdrawal of water at the same rate (and whatever temperature it is extracted at) at the right side:

!listing single_fracture_heat_transfer/fracture_app.i start=left_injection end=[]

!listing single_fracture_heat_transfer/fracture_app.i block=BCs

The physics in the matrix is assumed to be the simple heat equation.  Note that this has no stabilization, so there are overshoots and undershoots in the solution.

!listing single_fracture_heat_transfer/matrix_app.i block=Kernels

The [Transfers](multiapp_fracture_flow_transfers.md) are identical to those used in the [diffusion-equation case](multiapp_fracture_flow_diffusion.md):

!listing single_fracture_heat_transfer/matrix_app.i block=Transfers

The fracture-matrix-fracture-matrix-etc time-stepping procedure is the same as described in the [primer](multiapp_fracture_flow_primer.md).
Remember that the MultiApp approach typically breaks the unconditional stability of MOOSE's implicit time-stepping scheme.  This is because usually the fracture physics is "frozen" while the matrix physics is evolving, and vice-versa.  This can easily lead to oscillations if the time-step is too big.  For example, start with a cold matrix and hot fracture.  When the fracture evolves, it could transfer a huge amount of heat energy to the cold matrix, since the matrix temperature is fixed.  During its evolution, the matrix starts hot, so transfers the heat back to the fracture.  This cycle continues.

After 100$\,$s of simulation, the matrix temperature is shown in [single_fracture_heat_transfer_final_matrix_T].  The evolution of the temperatures is shown in [single_fracture_heat_transfer_T].  Without any heat transfer, the fracture heats up to 100$^{\circ}$C, but when the fracture transfers heat energy to the matrix, the temperature evolution is retarded.

!media porous_flow/examples/multiapp_flow/single_fracture_heat_transfer_final_matrix_T.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=single_fracture_heat_transfer_final_matrix_T
	caption=Heat conduction into the matrix (the matrix mesh is shown)

!media porous_flow/examples/multiapp_flow/single_fracture_heat_transfer_T.mp4
	style=width:60%;margin:auto;padding-top:2.5%;
	id=single_fracture_heat_transfer_T
	caption=Temperature along the fracture, with heat transfer to the matrix and without heat transfer to the matrix.  Fluid at 100degC is injected into the fracture at its left end, and fluid is withdrawn at the right end.  With heat transfer, the matrix heats up a little.
