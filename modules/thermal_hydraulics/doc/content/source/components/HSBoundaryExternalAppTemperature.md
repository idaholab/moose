# HSBoundaryExternalAppTemperature

!syntax description /Components/HSBoundaryExternalAppTemperature

This component is the same as [HSBoundarySpecifiedTemperature.md] but uses
a temperature transferred from an external application into an auxiliary variable.

The temperature variable is added by this component on the subdomains of the heat structure with a variable type
defined by the [HeatConductionModel.md].

The boundary temperature is imposed weakly on the boundary defined by the
[!param](/Components/HSBoundaryExternalAppTemperature/boundary) parameter
using an [ADMatchedValueBC.md] nodal boundary condition.

!syntax parameters /Components/HSBoundaryExternalAppTemperature

!syntax inputs /Components/HSBoundaryExternalAppTemperature

!syntax children /Components/HSBoundaryExternalAppTemperature
