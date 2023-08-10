# ADOneD3EqnMomentumAreaGradient

!syntax description /Kernels/ADOneD3EqnMomentumAreaGradient

The area gradient term, a form loss, in the momentum equation strong form is:

!equation
-P \nabla A \cdot \vec{d}

where $\nabla A$ the area of the component, $P$ the pressure and $\vec{d}$ the direction of the flow
channel.

!alert note
In THM, most kernels are added automatically by components or flow models. This kernel is created by the
[FlowModelSinglePhase.md] to act inside components with single-phase fluid flow.

!syntax parameters /Kernels/ADOneD3EqnMomentumAreaGradient

!syntax inputs /Kernels/ADOneD3EqnMomentumAreaGradient

!syntax children /Kernels/ADOneD3EqnMomentumAreaGradient
