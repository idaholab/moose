# TwoPhaseFluidPropertiesIndependent

!syntax description /FluidProperties/TwoPhaseFluidPropertiesIndependent

This 2-phase fluid properties class takes as its parameters the names of its
two 1-phase fluid properties objects, thus allowing an arbitrary combination of
1-phase fluid properties objects to be used. This class disallows calling any
of the 2-phase fluid properties interfaces (for example, saturation temperature),
since this class assumes that the 2 phases are independent and thus should
not have any 2-phase properties. This class is useful for using in test problems
that ideally should reduce to two 1-phase test problems acting independently.

!syntax parameters /FluidProperties/TwoPhaseFluidPropertiesIndependent

!syntax inputs /FluidProperties/TwoPhaseFluidPropertiesIndependent

!syntax children /FluidProperties/TwoPhaseFluidPropertiesIndependent

!bibtex bibliography
