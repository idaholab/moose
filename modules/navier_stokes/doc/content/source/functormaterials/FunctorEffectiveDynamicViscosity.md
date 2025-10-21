# FunctorEffectiveDynamicViscosity ADFunctorEffectiveDynamicViscosity

!syntax description /FunctorMaterials/FunctorEffectiveDynamicViscosity

This class implements the effective dynamic viscosity used notably in turbulence models.
They are generally of the form:

!equation
\mu_{eff} = \mu_{dynamic} + \mu_{turbulence} / f

where $\mu$ are viscosity terms, and $f$ is a functor which depends on the equation being considered.
An additional real-valued scaling factor may also be used to scale the turbulent viscosity.

!syntax parameters /FunctorMaterials/FunctorEffectiveDynamicViscosity

!syntax inputs /FunctorMaterials/FunctorEffectiveDynamicViscosity

!syntax children /FunctorMaterials/FunctorEffectiveDynamicViscosity
