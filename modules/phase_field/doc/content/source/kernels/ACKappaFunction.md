# ACKappaFunction

The ACKAppaFunction Kernel calculates the term
$\frac12 L \frac{\partial \kappa}{\partial \eta} f_g(\nabla \eta)$
for the case where $\kappa$ is a function of $\eta$ and $f_g$ is the gradient
energy function used in the phase field method.

## Description

### General Information

This kernel should only be used for variables for which $\kappa$ has derivatives.
Otherwise it wastes computational resources.

$\kappa$ and its derivatives should be calculated in a material block which uses
the ```DerivativeMaterialInterface```, such as ```DerivativeParsedMaterial```.

!syntax parameters /Kernels/ACKappaFunction

!syntax inputs /Kernels/ACKappaFunction

!syntax children /Kernels/ACKappaFunction
