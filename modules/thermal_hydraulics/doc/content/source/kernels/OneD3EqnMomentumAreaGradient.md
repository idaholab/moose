# OneD3EqnMomentumAreaGradient

!syntax description /Kernels/OneD3EqnMomentumAreaGradient

The area gradient term, a form loss, in the momentum equation strong form is:

!equation
-P \nabla A \cdot \vec{d}

where $\nabla A$ the area of the component, $P$ the pressure and $\vec{d}$ the direction of the flow
channel.

!alert note
In THM, most kernels are added automatically by components. This kernel is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADOneD3EqnMomentumAreaGradient.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /Kernels/OneD3EqnMomentumAreaGradient

!syntax inputs /Kernels/OneD3EqnMomentumAreaGradient

!syntax children /Kernels/OneD3EqnMomentumAreaGradient
