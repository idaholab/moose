# FormLossFromExternalApp1Phase

!syntax description /Components/FormLossFromExternalApp1Phase

This form loss follows the model used in [FormLoss1PhaseBase.md],
it simply defines the loss coefficient as being `K_prime`, a first order
Lagrange variable defined over the component block that will be populated from
an external application. This variable will then be converted to a material property
using an [CoupledVariableValueMaterial.md], to be used by the kernel added by [FormLoss1PhaseBase.md]. 

!syntax parameters /Components/FormLossFromExternalApp1Phase

!syntax inputs /Components/FormLossFromExternalApp1Phase

!syntax children /Components/FormLossFromExternalApp1Phase
