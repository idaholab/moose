# WeightedVariableAverage

!syntax description /Postprocessors/WeightedVariableAverage

Compute the ratio $a$ of volume integrals
\begin{equation}
a=\frac{\int_Omega w\cdot v dr}{\int_Omega w dr},
\end{equation}
where $v$ (`v`) is a coupled variable and $w$ (`weight`) a material property.

For constant weight values $w$ this object is equivalent to
[ElementAverageValue](/ElementAverageValue.md).

!syntax parameters /Postprocessors/WeightedVariableAverage

!syntax inputs /Postprocessors/WeightedVariableAverage

!syntax children /Postprocessors/WeightedVariableAverage

!bibtex bibliography
