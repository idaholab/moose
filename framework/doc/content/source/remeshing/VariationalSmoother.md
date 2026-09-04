# VariationalSmoother

!syntax description /Remeshing/Smoothers/VariationalSmoother

## Description

This smoother adds a corrector to [LaplaceSmoother.md]. The harmonic solve of that class is the
predictor: it carries the prescribed motion of the moving boundaries into the interior and along the
sliding walls, so the configuration handed to the corrector is untangled even when a moving boundary
travels along a wall whose nodes it would otherwise overtake. The corrector is the variational mesh
smoother of libMesh, which relocates the nodes of the predicted configuration to minimize the
distortion-dilation metric of [!cite](branets2005variationalgrid) summed over the elements,

\begin{equation}
E = (1 - w) \, E_{\mathrm{distortion}} + w \, E_{\mathrm{dilation}} ,
\end{equation}

where $E_{\mathrm{distortion}}$ measures how far each element departs from the shape of its
reference element, $E_{\mathrm{dilation}}$ measures how far the volume of each element departs from
the mean element volume of the mesh, and $w$ is
[!param](/Remeshing/Smoothers/VariationalSmoother/dilation_weight). A weight of zero optimizes the
element shapes alone and lets the element sizes drift apart. A weight of one equalizes the element
volumes alone and lets the shapes distort. The default of one half weights the two equally.

The corrector constrains every boundary node from the current geometry. A node on a flat stretch of
boundary slides along it, and a node at a corner is pinned where it sits, so the boundary keeps the
shape the predictor prescribed and only the interior nodes move freely. The pseudo-displacement the
smoother writes is the total motion of every node from the reference configuration, read back off
the mesh after the corrector has run, so it obeys the same reference-configuration contract as
[LaplaceSmoother.md]. A corrector solve that fails to converge ends the run with an error.

Every parameter of [LaplaceSmoother.md] applies, and the mesh must be first order for the same
reason.

## Example Input Syntax

!listing test/tests/remeshing/demos/distributed_wall_sweep.i block=Remeshing

!syntax parameters /Remeshing/Smoothers/VariationalSmoother

!syntax inputs /Remeshing/Smoothers/VariationalSmoother

!syntax children /Remeshing/Smoothers/VariationalSmoother
