# NSMixtureMaterial

!syntax description /Materials/NSMixtureMaterial

This material is mainly used in multiphase modeling.
Given a phase fraction functor [!param](/Materials/NSMixtureMaterial/phase_1_fraction) ($\lambda_1$)
and two vectors of properties [!param](/Materials/NSMixtureMaterial/phase_1_names) and
[!param](/Materials/NSMixtureMaterial/phase_2_names), named generically $p_{i,1}$ and $p_{i,2}$, respectively,
the material computes the phase-weighted average of the property as follows:

\begin{equation}
p_i = \lambda_1 p_{i,1} + (1 - \lambda_1) p_{i,2} \,,
\end{equation}

where we assume that $\lambda_2 = 1 - \lambda_1$.
The name of the output properties can be defined using the [!param](/Materials/NSMixtureMaterial/prop_names) parameter.

!syntax parameters /Materials/NSMixtureMaterial

!syntax inputs /Materials/NSMixtureMaterial

!syntax children /Materials/NSMixtureMaterial
