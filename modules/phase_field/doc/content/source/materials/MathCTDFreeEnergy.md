# MathCTDFreeEnergy

!syntax description /Materials/MathCTDFreeEnergy

Implements the bulk free free energy density

!equation
f_\text{bulk}(c) = \frac14(1-c)^2(1+c)^2

using the [CompileTimeDerivatives](/CompileTimeDerivatives.md) system, where $c$ (`c`) is
a coupled concentration. The minima of this free energy density lie at $c=1$ and
$c=-1$. See also [`MathEBFreeEnergy`](/MathEBFreeEnergy.md).

!syntax parameters /Materials/MathCTDFreeEnergy

!syntax inputs /Materials/MathCTDFreeEnergy

!syntax children /Materials/MathCTDFreeEnergy
