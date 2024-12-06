# INSADSurfaceTensionBC

This boundary condition implements effects of surface tension on a free
surface using the following expression:

\begin{equation}
\vec{n}\cdot \sigma_{liq} = -2 \mathcal{H} \sigma \vec{n} - \nabla_s \sigma \,,
\end{equation}

where $\mathcal{H}$ is the mean curvature of the surface, while
$\sigma$ describes the suface tension. In this context, $\nabla_s = (I-\vec{n}\vec{n})\cdot \nabla$
is the surface gradient operator. Parameter [!param](/BCs/INSADSurfaceTensionBC/include_gradient_terms)
can be used to enable or disable the second term in the expression.
Disabling the second term would disable the Marangoni effect and would decrease the
surface deformations. This decreases the fidelity of the model, with an increased robustness.
The model is based on the one discussed in [!cite](cairncross2000finite).

!syntax parameters /BCs/INSADSurfaceTensionBC

!syntax inputs /BCs/INSADSurfaceTensionBC

!syntax children /BCs/INSADSurfaceTensionBC
