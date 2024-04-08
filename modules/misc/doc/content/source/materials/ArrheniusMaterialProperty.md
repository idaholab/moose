# ArrheniusMaterialProperty

!syntax description /Materials/ArrheniusMaterialProperty

## Description

`ArrheniusMaterialProperty` is used to declare an arbitrary material property $D$. For example, mass
diffusion coefficients are typically defined using an Arrhenius form [!citep](rpt:miller:2009)
\begin{equation}
D(T) = \sum_i D_{0,i}\exp{ \left( \frac{-Q_{i}}{RT} \right)}
\end{equation}
where $D_{0,i}$ is a pre-exponential factor, $Q_i$ is the activation energy, $R$ is the
universal gas constant, and $T$ is temperature. Also included is the derivative of $D$ with respect
to temperature.

## Example Input Syntax

!listing test/tests/arrhenius_material_property/exact.i block=Materials/D

!syntax parameters /Materials/ArrheniusMaterialProperty

!syntax inputs /Materials/ArrheniusMaterialProperty

!syntax children /Materials/ArrheniusMaterialProperty

!bibtex bibliography
