# HeatRateConvection1Phase

This post-processor computes the total heat rate due to convection on a
1-phase flow channel:
\begin{equation}
  \dot{Q} = \int \mathcal{H} (T_\text{wall} - T) P_\text{hf} dx \,.
\end{equation}

!syntax parameters /Postprocessors/HeatRateConvection1Phase

!syntax inputs /Postprocessors/HeatRateConvection1Phase

!syntax children /Postprocessors/HeatRateConvection1Phase

!tag name=HeatRateConvection1Phase pairs=module:thermal_hydraulics system:postprocessors
