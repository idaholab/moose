# RadialDisplacementCylinderAux

!syntax description /AuxKernels/RadialDisplacementCylinderAux

Computing the radial displacement for axisymmetric models is simply a matter
of reporting $u_r$.

For a 2D Cartesian model, the center axis is in the out-of-plane direction.  In this case, the vector from the origin to a node is $p_{n0} = p_n - p_0$.  The radial displacement is then $u_r = u_n \cdot \frac{p_{n0}}{\left\lVert{p_{n0}}\right\rVert}$.

For a 3D Cartesian model, we first find the point on the axis of rotation that is closest to a particular node.  This is done by
\begin{equation}
  p_{n0} = p_n - p_0
\end{equation}
\begin{equation}
  d = p_{n0} \cdot a_r
\end{equation}
\begin{equation}
  p = p_0 + d a_r
\end{equation}
\begin{equation}
  p_r = p_n - p
\end{equation}
where $a_r$ is the axis of rotation.
Then
\begin{equation}
u_r = u_n \cdot \frac{p_{r}}{\left\lVert{p_{r}}\right\rVert}.
\end{equation}

!syntax parameters /AuxKernels/RadialDisplacementCylinderAux

!syntax inputs /AuxKernels/RadialDisplacementCylinderAux

!syntax children /AuxKernels/RadialDisplacementCylinderAux

!bibtex bibliography
