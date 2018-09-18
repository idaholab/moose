# CrossTermBarrierFunctionMaterial

!syntax description /Materials/CrossTermBarrierFunctionMaterial

The material provides a function $g(\vec\eta)$ that is parameterized by all
phase order parameters $\eta$. It calculates a phase transformation barrier energy
that contains contributions for pairs of order parameters.

With the `g_order` parameter set to `SIMPLE` the function is defined as

\begin{equation}
g(\vec\eta) = 16\sum_i\sum_{j>i} W_{ij} \eta_i^2\eta_j^2
\end{equation}

and with the  `g_order` parameter set to `LOW` it is defined as

\begin{equation}
g(\vec\eta) = 16\sum_i\sum_{j>i} W_{ij} \eta_i\eta_j
\end{equation}

The $W_{ij}$ is a matrix of all coefficients for the pair terms.

!syntax parameters /Materials/CrossTermBarrierFunctionMaterial

!syntax inputs /Materials/CrossTermBarrierFunctionMaterial

!syntax children /Materials/CrossTermBarrierFunctionMaterial
