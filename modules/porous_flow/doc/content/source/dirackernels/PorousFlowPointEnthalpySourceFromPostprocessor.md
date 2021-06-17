# PorousFlowPointEnthalpySourceFromPostprocessor

`PorousFlowPointEnthalpySourceFromPostprocessor` implements a point source that adds heat energy
corresponding to adding fluid at a mass flux rate (computed by a postprocessor) at a specified
temperature (computed by a postprocessor).

This object should be used in conjunction with (PorousFlowPointSourceFromPostprocessor)[PorousFlowPointSourceFromPostprocessor.md]
that uses the same `mass_flux` Postprocessor, so that the correct amount of fluid is injected into the system.

Note that the fluid property object used by this Dirac kernel should be the same one that used in
the computational domain where this object is located.

Parameter [!param](/DiracKernels/PorousFlowPointEnthalpySourceFromPostprocessor/pressure)
(along with [!param](/DiracKernels/PorousFlowPointEnthalpySourceFromPostprocessor/T_in)) is used to calculate
the injected fluid enthalpy. Most frequently, it is the PorousFlow pressure variable (the porepressure
in the porous medium). This models the situation where fluid is injected at a specified rate and
temperature (using this DiracKernel and a PorousFlowPointSourceFromPostprocessor) which potentially
leads to changes in the porepressure. Alternately, it may be the pressure of the injected fluid, as
fixed by an external agent (such as a pump) which is stored in an AuxVariable.

For instance:

!listing modules/porous_flow/test/tests/dirackernels/hfrompps.i block=DiracKernels

Note that the ```execute_on``` parameter is set to ```timestep_begin``` so that the correct
value is being used within the timestep.

!listing modules/porous_flow/test/tests/dirackernels/hfrompps.i block=Postprocessors

!listing modules/porous_flow/test/tests/dirackernels/hfrompps.i block=DiracKernels

!syntax parameters /DiracKernels/PorousFlowPointEnthalpySourceFromPostprocessor

!syntax inputs /DiracKernels/PorousFlowPointEnthalpySourceFromPostprocessor

!syntax children /DiracKernels/PorousFlowPointEnthalpySourceFromPostprocessor
