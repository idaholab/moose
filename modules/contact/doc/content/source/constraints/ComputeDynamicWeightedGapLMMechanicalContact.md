# ComputeDynamicWeightedGapLMMechanicalContact

This object is virtually analogous to [`ComputeWeightedGapLMMechanicalContact`](/ComputeWeightedGapLMMechanicalContact.md), but it is used for dynamic simulations with the Newmark-beta integrator. 

When the mortar mechanical contact constraints are used in dynamic simulations, the normal contact constraints need to be stabilized by ensuring that normal gap derivatives are included in the definition. This is a way of guaranteeing the 'persistency' condition, i.e. not only do we enforce the constraints instantaneously, but also that it will remain in contact. This approximate contact constraint stabilization is performed in  [`ComputeDynamicWeightedGapLMMechanicalContact`](/ComputeDynamicWeightedGapLMMechanicalContact.md), where the gap is defined as 

\begin{equation}
\begin{aligned}
(\tilde{g}_n)_j \coloneqq (\tilde{g}_n)_j - 0.5 (\cdot (\dot{\tilde{g}}_{n})_j \cdot \Delta t + \beta \cdot (\ddot{\tilde{g}}_{n})_{j} \cdot \Delta t^{2})
\end{aligned}
\end{equation}

where $\,\dot{}\,$ and $\,\ddot{}\,$ denote first and second order time derivatives, respectively, $\Delta t$ is the time step, and $\beta$ is a Newmark-beta integrator parameter.


!syntax description /Constraints/ComputeDynamicWeightedGapLMMechanicalContact

!syntax parameters /Constraints/ComputeDynamicWeightedGapLMMechanicalContact

!syntax inputs /Constraints/ComputeDynamicWeightedGapLMMechanicalContact

!syntax children /Constraints/ComputeDynamicWeightedGapLMMechanicalContact