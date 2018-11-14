# ACGrGrMulti

!syntax description /Kernels/ACGrGrMulti

Implements the term

\begin{equation}
L \mu \left( \eta_{\alpha i}^3 - \eta_{\alpha i} + 2\eta_{\alpha i} \sum_{\beta=1}^N
\sum_{j=1, \alpha i \neq \beta j}^{p_\beta} \gamma_{\alpha i \beta j} \eta_{\beta j}^2 \right)
\end{equation}

for a multiphase / multigrain Grand Potential model. The parameters $\mu$
(hardcoded material property name `mu`) and $\gamma$ (`gamma_names`) can be
calculated using the [`GrandPotentialInterface`](/GrandPotentialInterface.md)
material.

$L$ (`mob_name`) is an interfacial mobility, $\eta_{\alpha i}$ (`variable`) is
the kernel variable, and $\eta_{\beta j}$ (`v`) are the coupled phase / grain
order parameters. If $L$ is a [Function Material](/FunctionMaterials.md)
depending on any non-linear variable not listed in `v` those will have to be
listed in `args`.

!syntax parameters /Kernels/ACGrGrMulti

!syntax inputs /Kernels/ACGrGrMulti

!syntax children /Kernels/ACGrGrMulti

!bibtex bibliography
