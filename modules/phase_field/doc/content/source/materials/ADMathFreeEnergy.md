# ADMathFreeEnergy

This material implements an analytical double well free energy

\begin{equation}
F = \frac14(1 + c)^2(1 - c)^2
\end{equation}

and its first order derivatives. Both provided properties furthermore come with
forward mode automatic derivatives with respect to all degrees of freedom on the
current element.

This free energy can be used with the AD version of the split Cahn-Hilliard
equation ([`ADSplitCHWRes`](/ADSplitCHWRes.md) and
[`ADSplitCHParsed`](/ADSplitCHParsed.md)).

!syntax parameters /Materials/ADMathFreeEnergy

!syntax inputs /Materials/ADMathFreeEnergy

!syntax children /Materials/ADMathFreeEnergy

!bibtex bibliography
