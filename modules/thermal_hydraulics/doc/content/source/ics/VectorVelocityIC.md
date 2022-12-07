# VectorVelocityIC

This object computes a component of a vector-valued velocity in the direction of a 1-D element from a scalar velocity function.

It computes:
\begin{equation}
u_i = u_d d_i
\end{equation}
where $u$ is the velocity vector, $u_d$ is user-provided function and $d$ is the direction
of the element where this IC is being applied.

!syntax parameters /ICs/VectorVelocityIC

!syntax inputs /ICs/VectorVelocityIC

!syntax children /ICs/VectorVelocityIC

!bibtex bibliography
