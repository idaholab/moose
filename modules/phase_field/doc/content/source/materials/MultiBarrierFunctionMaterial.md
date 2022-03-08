# MultiBarrierFunctionMaterial

!syntax description /Materials/MultiBarrierFunctionMaterial

The material provides a function $g(\vec\eta)$ that is parameterized by all
phase order parameters $\eta$. It calculates a phase transformation barrier
energy that contains double well contributions for each single order parameter.

Currently the `g_order` only supports the `SIMPLE` setting and the function is
defined as

\begin{equation}
g(\vec\eta) = \sum_i \eta_i^2(1-\eta_i)^2.
\end{equation}

!syntax parameters /Materials/MultiBarrierFunctionMaterial

!syntax inputs /Materials/MultiBarrierFunctionMaterial

!syntax children /Materials/MultiBarrierFunctionMaterial
