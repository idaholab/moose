# MathEBFreeEnergy

!syntax description /Materials/MathEBFreeEnergy

Implements the bulk free free energy density

!equation
f_\text{bulk}(c) = \frac14(1-c)^2(1+c)^2

using the [ExpressionBuilder](/ExpressionBuilder.md) system, where $c$ (`c`) is
a coupled concentration. The minima of this free energy density lie at $c=1$ and
$c=-1$. See also [`SplitCHMath`](/SplitCHMath.md).

!syntax parameters /Materials/MathEBFreeEnergy

!syntax inputs /Materials/MathEBFreeEnergy

!syntax children /Materials/MathEBFreeEnergy
