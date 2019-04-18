# ADThermoDiffusion

## Description

`ADThermoDiffusion` implements thermodiffusion (also called thermophoresis, thermomigration, or the Soret effect)
or the movement of species due a temperature gradient. The mass flux $J$
is given as
\begin{equation}
\mathbf{J}=-S\nabla T
\end{equation}
where $S$ is Soret diffusion coefficient which is typically a combination of the
species concentration, temperature, and other material parameters. $S$ is kept
agnostic in this formulation and ultimately needs to be formulated in a separate
material property definition.

!syntax parameters /Kernels/ADThermoDiffusion

!syntax inputs /Kernels/ADThermoDiffusion

!syntax children /Kernels/ADThermoDiffusion

!bibtex bibliography
