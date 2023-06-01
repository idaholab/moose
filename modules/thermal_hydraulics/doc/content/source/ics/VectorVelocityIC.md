# VectorVelocityIC

This object computes a component of a vector-valued velocity in the direction of a 1-D element from a scalar velocity function.

It computes:
\begin{equation}
u_i = u_d d_i
\end{equation}
where $u$ is the velocity vector, $\mathbf{d}$ is the unit direction vector
of the element where this IC is being applied,
and $u_d$ is a user-provided function for the component of velocity in the direction $\mathbf{d}$.

!syntax parameters /ICs/VectorVelocityIC

!syntax inputs /ICs/VectorVelocityIC

!syntax children /ICs/VectorVelocityIC
