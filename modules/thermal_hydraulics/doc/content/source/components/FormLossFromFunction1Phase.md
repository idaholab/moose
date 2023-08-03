# FormLossFromFunction1Phase

!syntax description /Components/FormLossFromFunction1Phase

This form loss follows the model used in [FormLoss1PhaseBase.md];
it simply defines the loss coefficient as being the function passed to the [!param](/Components/FormLossFromFunction1Phase/K_prime). This `Function` will then be converted to a material property using an
[GenericFunctionMaterial.md], to be used by the kernel added by [FormLoss1PhaseBase.md].

!syntax parameters /Components/FormLossFromFunction1Phase

!syntax inputs /Components/FormLossFromFunction1Phase

!syntax children /Components/FormLossFromFunction1Phase
