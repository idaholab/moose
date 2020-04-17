# PorousFlowPointSourceFromPostprocessor

`PorousFlowPointSourceFromPostprocessor`
implements a mass point source that adds (or removes) fluid at a
mass flux rate that was computed by a postprocessor.

For instance:

!listing modules/porous_flow/test/tests/dirackernels/frompps.i block=DiracKernels

Note that the ```execute_on``` parameter is set to ```timestep_begin``` so that the correct
value is being used within the timestep.

!listing modules/porous_flow/test/tests/dirackernels/frompps.i block=Postprocessors

!syntax parameters /DiracKernels/PorousFlowPointSourceFromPostprocessor

!syntax inputs /DiracKernels/PorousFlowPointSourceFromPostprocessor

!syntax children /DiracKernels/PorousFlowPointSourceFromPostprocessor
