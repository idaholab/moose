# PorousFlowHystereticInfo

This Material computes quantities relevant to [hysteretic capillary pressure](hysteresis.md).  The quantity computed is written into the `PorousFlow_hysteretic_info_qp` or `PorousFlow_hysteretic_nodal` Property (depending on the `at_nodes` parameter).

!alert warning
This Material does not compute porepressures or saturations, so cannot be used in usual PorousFlow simulations.  Instead, it enables preliminary exploration of hysteresis in models before hysteretic PorousFlow simulations are run: informative plots may be generated using this Material.

The quantity computed depends on the `info_required` parameter, which may be one of the following.

- `pc` capillary pressure, given the saturation
- `sat` saturation.  The saturation is provided as an input, the capillary pressure is computed, and then the saturation is computed using the inverse relationship.  This is useful when exploring th enon-invertibility of the hysteretic relationships
- `sat_given_pc` saturation, given the capillary pressure
- various derivative information, such as the numerical error in $dP_{c}/dS$, etc.

An example input file containing `PorousFlowHystereticInfo` is

!listing modules/porous_flow/test/tests/hysteresis/vary_sat_1.i

In this input file, saturation is an AuxVariable which is varied with time through a `FunctionAux`.  The `PorousFlowHystereticInfo` Material computes capillary pressure, which is written into an AuxVariable using the [PorousFlowPropertyAux](PorousFlowPropertyAux.md) AuxKernel.  By choosing different `FunctionAux`, various capillary-pressure curves may be generated such as the one shown below.

!media media/porous_flow/hys_vary_sat_1.png caption=The results of two hysteretic simulations employing different `FunctionAux`.  The lines show the expected result while the crosses and asterisks show the MOOSE result.



!syntax parameters /Materials/PorousFlowHystereticInfo

!syntax inputs /Materials/PorousFlowHystereticInfo

!syntax children /Materials/PorousFlowHystereticInfo
