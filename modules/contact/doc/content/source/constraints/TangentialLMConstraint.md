# TangentialLMConstraint

The `TangentialLMConstraint` enforces the Coulomb model of frictional
contact. It enforces the following Karush-Kuhn-Tucker conditions:

\begin{equation}\label{eq:kkt}
\begin{aligned}
    \dot{\gamma} &\geq 0\\
    -f(\vec{t}_T) &\geq 0\\
    \dot{\gamma}f(\vec{t}_T) &= 0
\end{aligned}
\end{equation}

where:

\begin{equation}
\begin{aligned}
    \vec{v}_T &= \dot{\gamma}\frac{\vec{t}_T}{||\vec{t}_T||}\\
    f(\vec{t}_T) &= ||\vec{t}_T|| - \mu p_N
\end{aligned}
\end{equation}

and $\vec{v}_T$ is the tangential velocity, $\vec{t}_T$ is the tangential
stress, $\mu$ is the friction coefficient, and $p_N$ is the normal contact
pressure. The KKT conditions are implemented using a Nonlinear Complimentarity
Function (NCP), specifically the Fischer-Burmeister function:

\begin{equation}
    \phi_{FB}(a, b) = a + b - \sqrt{a^2 + b^2}
\end{equation}

where $a$ corresponds to the LHS on the first line in [eq:kkt] and $b$
corresponds to the LHS on the second. In two dimensions, $\vec{t}_T$ can be
thought of as $t_T\hat{a}$ where $\hat{a}$ is a unit vector in the direction of
the surface tangent. $t_T$ is a Lagrange Multiplier that has both magnitude and
direction represented by its sign. If positive, then the tangential stress
exerted by friction on the slave node points in the direction of the tangent
vector; if negative, then it points in the opposite direction. $t_T$ is an
additional variable in the non-linear system.

## Summary

!syntax description /Constraints/TangentialLMConstraint

!syntax parameters /Constraints/TangentialLMConstraint

!syntax inputs /Constraints/TangentialLMConstraint

!syntax children /Constraints/TangentialLMConstraint

!bibtex bibliography
