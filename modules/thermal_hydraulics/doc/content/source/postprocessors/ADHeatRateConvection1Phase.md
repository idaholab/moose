# ADHeatRateConvection1Phase

!syntax description /Postprocessors/ADHeatRateConvection1Phase

The heat convective heat rate $R$ is the integral of the convective heat flux over the flow channel / subdomains
specified in the [!param](/Postprocessors/ADHeatRateConvection1Phase/block) parameter.

!equation
R = \int_\Omega H_{wall} P_w (T - T_{wall}) d\Omega

where $H_w$ is the wall heat transfer coefficient, $P_w$ is the wetted perimeter,
$T$ is the fluid temperature and $T_{wall}$ is the wall temperature.

!syntax parameters /Postprocessors/ADHeatRateConvection1Phase

!syntax inputs /Postprocessors/ADHeatRateConvection1Phase

!syntax children /Postprocessors/ADHeatRateConvection1Phase
