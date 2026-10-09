# MeshMotionCriterion

!syntax description /Remeshing/Criteria/MeshMotionCriterion

## Description

The measured quantity is, over the active elements $e$ of the mesh,

\begin{equation}
\max_e \frac{\max_{n \in e} \lVert \mathbf{d}_n \rVert}{h_e} ,
\end{equation}

where $\mathbf{d}_n$ is the pseudo-displacement of node $n$ accumulated since the last mesh
replacement and $h_e$ is the largest vertex separation of element $e$, which is its diameter for a
straight-sided element. The criterion fires when that ratio, reduced over all ranks, exceeds
[!param](/Remeshing/Criteria/MeshMotionCriterion/threshold). Normalizing by the element size makes
the threshold a fraction of the local mesh size rather than an absolute length, so one value suits a
graded mesh.

The element size is measured on the displaced mesh when
[!param](/Remeshing/RemeshingAction/displacements) names the displacement variables of the problem,
and on the reference mesh otherwise.

The pseudo-displacement is identically zero unless
[!param](/Remeshing/RemeshingAction/mesh_movement) is `true`, so this criterion requires it and
reports an error during setup when it is `false`.

## Example Input File Syntax

!listing test/tests/remeshing/ale_remesh_reset.i block=Remeshing

!syntax parameters /Remeshing/Criteria/MeshMotionCriterion

!syntax inputs /Remeshing/Criteria/MeshMotionCriterion

!syntax children /Remeshing/Criteria/MeshMotionCriterion
