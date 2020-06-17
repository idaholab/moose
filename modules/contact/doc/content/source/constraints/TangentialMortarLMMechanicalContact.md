# TangentialMortarLMMechanicalContact

The `TangentialMortarLMMechanicalContact` class is used to ensure that the
Karush-Kuhn-Tucker conditions of Coulomb frictional contact are satisfied:

\begin{equation}
\begin{aligned}
\phi &= \mu\lambda_N - \lvert \lambda_T \rvert \ge 0\\
\gamma &= \frac{v_T}{\lambda_T} \ge 0\\
\gamma\phi &= 0
\end{aligned}
\end{equation}

where $\mu$ is the coefficient of friction, $\lambda_N$ is the Lagrange
multiplier variable representing the contact pressure, $v_T$ is the slip
velocity of the secondary face relative to the primary face, and $\lambda_T$ is the
Lagrange multiplier variable representing the tangential stress. The above
conditions require that either the secondary face is sticking to the primary face, or
the secondary face is slipping and the tangential stress is equal in magnitude to
the coefficient of friction times the normal contact pressure. Additionally, if
the face is slipping, then the force exerted by the secondary face is in the same
direction as the slip.

The `ncp_function_type` parameter specifies the type of nonlinear
complimentarity problem (NCP) function to use. The options are either `min`, which is just the
min function, or `fb` which represents the Fischer-Burmeister function. In our
experience, the Fischer-Burmeister function achieves better convergence in the
non-linear solve. The `c` parameter is used to balance the size of the slip
velocity and the tangential stress. If the stress is of order 1000, and the
velocity is of order 1, then `c` should be set to 1000 in order to bring
components of the NCP function onto the same level and achieve optimal
convergence in the non-linear solve.

!syntax description /Constraints/TangentialMortarLMMechanicalContact

!syntax parameters /Constraints/TangentialMortarLMMechanicalContact

!syntax inputs /Constraints/TangentialMortarLMMechanicalContact

!syntax children /Constraints/TangentialMortarLMMechanicalContact
