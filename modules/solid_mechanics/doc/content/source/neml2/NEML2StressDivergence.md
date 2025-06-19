# NEML2StressDivergence

For a batch of material points, calculate the residual at each point in the form of $\phi^\alpha_{,j} \sigma_{ij}$, for $\alpha = 1..n$ where $n$ is the number of displacement variables.

The kernel then assembles the integrated residual into the global residual vector.

## Syntax

!syntax parameters /UserObjects/NEML2StressDivergence

## Example input files

!syntax inputs /UserObjects/NEML2StressDivergence
