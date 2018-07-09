# LangevinNoise

!syntax description /Kernels/LangevinNoise

Adds a uniform noise source term with the weak form
\begin{equation}
(2\chi-1)\alpha P(\vec r)\psi,
\end{equation}
where $\chi$ is a random number in $[0\dots1)$, $\alpha$ (`amplitude`) is a constant
amplitude factor, and $P(\vec r)$ (`multiplier`) is a material property that can be used
as a location dependent mask for the source term.

!alert warning
This kernel does not produce parallel reproducible results. For a parallel reproducible random
noise system see [`ConservedLangevinNoise`](/ConservedLangevinNoise.md)

!syntax parameters /Kernels/LangevinNoise

!syntax inputs /Kernels/LangevinNoise

!syntax children /Kernels/LangevinNoise
