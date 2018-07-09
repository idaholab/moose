# EquilibriumConstantAux

!syntax description /AuxKernels/EquilibriumConstantAux

## Description

Calculates the equilibrium constant $K_{eq}$ given a series of temperature - $\log_{10}(K_eq)$
points, which is returned as $\log_{10}(K_eq)$.

If more than five points are provided, $\log_{10}(K_eq)$ is calculated using a Maier-Kelly
type fit

\begin{equation}
\log_{10}(K_{eq}) = c_1 \log(T) + c_2 + c_3 T + c_4/T + c_5/T^2,
\end{equation}
where $T$ is the temperature in Kelvin.

If between two and four data points are provided, a linear least-squares fit is used
to calculate $\log_{10}(K_eq)$.

If only one data point is provided, then $\log_{10}(K_eq)$ is set as a constant value.

## Example Input File Syntax

!listing modules/chemical_reactions/test/tests/equilibrium_const/maier_kelly.i block=AuxKernels

!syntax parameters /AuxKernels/EquilibriumConstantAux

!syntax inputs /AuxKernels/EquilibriumConstantAux

!syntax children /AuxKernels/EquilibriumConstantAux
