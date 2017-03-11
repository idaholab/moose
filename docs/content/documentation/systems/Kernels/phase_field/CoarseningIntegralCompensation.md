# CoarseningIntegralCompensation
!description /Kernels/CoarseningIntegralCompensation

As coarsening drops nodes in regions with high curvature the integrals over variables
can be non-conserved. The integral change due to coarsening can be monitored using
the [CoarseningIntegralTracker](/CoarseningIntegralTracker.md) user object,
and the monitored change can be subtracted using this source term kernel to restore
conservation of the integral.

!parameters /Kernels/CoarseningIntegralCompensation

!inputfiles /Kernels/CoarseningIntegralCompensation

!childobjects /Kernels/CoarseningIntegralCompensation
