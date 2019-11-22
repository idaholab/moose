# PolycrystalDislocationKernelAction

!syntax description /Kernels/PolycrystalDislocationKernel/PolycrystalDislocationKernelAction

The `PolycrystalDislocationKernelAction` is an action that sets up kernels for a grain growth
simulation with the addition of a bulk free energy term based on dislocation density.  The
grain growth setup proceeds via the [PolycrystalKernelAction](/PolycrystalKernelAction.md) and
the free energy term for each order parameter is added via the [ACPolycrystalDislocationEnergy](ACPolycrystalDislocationEnergy.md) kernel.

!syntax parameters /Kernels/PolycrystalDislocationKernel/PolycrystalDislocationKernelAction

!syntax inputs /Kernels/PolycrystalDislocationKernel/PolycrystalDislocationKernelAction

!syntax children /Kernels/PolycrystalDislocationKernel/PolycrystalDislocationKernelAction

!bibtex bibliography
