# ComputeDynamicWeightedGapLMMechanicalContact

!syntax description /Constraints/ComputeDynamicWeightedGapLMMechanicalContact

This object is virtually analogous to [`ComputeWeightedGapLMMechanicalContact`](/ComputeWeightedGapLMMechanicalContact.md), but it is used for dynamic simulations. 

When the mortar mechanical contact constraints are used in dynamic simulations, the normal contact constraints need to be stabilized by ensuring that the normal gap time derivative is included to guarantee a dynamic contact. This is means of guaranteeing the 'persistency' condition, i.e. not only do we enforce the constraints instantaneously, but also that it will remain in contact. This approximate contact constraint stabilization is performed in  [`ComputeDynamicWeightedGapLMMechanicalContact`](/ComputeDynamicWeightedGapLMMechanicalContact.md). Once nodal contact is established, the constraint enforcement is switched to the persistency equation: 

\begin{equation}
\begin{aligned}
(\tilde{g}_n)_j \coloneqq  \dot{\tilde{g}}_{nj} \cdot \Delta t
\end{aligned}
\end{equation}

where $\,\dot{}\,$ denotes the first time derivative, $\Delta t$ is the time step increment, $j$ refers to an arbitrary node.

The `capture_tolerance` is an optional contact parameter used in dynamic contact constraints to determine when to impose the persistency condition for normal contact. For relevant, general equations, see [!citep](tal2018dynamic).

!syntax parameters /Constraints/ComputeDynamicWeightedGapLMMechanicalContact

!syntax inputs /Constraints/ComputeDynamicWeightedGapLMMechanicalContact

!syntax children /Constraints/ComputeDynamicWeightedGapLMMechanicalContact