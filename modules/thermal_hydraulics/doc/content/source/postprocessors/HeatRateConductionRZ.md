# HeatRateConductionRZ

This post-processor computes the conduction heat rate $\dot{Q}$ across a radial
surface of 2D RZ domain (this post-processor inherits from [RZSymmetry.md]).

The conduction heat rate across a surface $S$ is computed as

!equation
\dot{Q} = \int\limits_S -f_\text{dir} k \nabla T \cdot \mathbf{n} \, dA \,,

where

- $f_\text{dir}$ is the "direction factor", equal to 1 if computing the *outward*
  heat rate and -1 if computing the *inward* heat rate,
- $k$ is the thermal conductivity,
- $T$ is the temperature, and
- $\mathbf{n}$ is the outward normal unit vector of $S$.

!syntax parameters /Postprocessors/HeatRateConductionRZ

!syntax inputs /Postprocessors/HeatRateConductionRZ

!syntax children /Postprocessors/HeatRateConductionRZ
