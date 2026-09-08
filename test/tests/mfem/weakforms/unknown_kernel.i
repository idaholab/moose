# Include mfem/kernels/darcy.i
!include ../kernels/darcy.i

[WeakForms]
  [Darcy]
    type = MFEMWeakForm
    kernels = 'NotAKernel'
    bcs = 'flux_boundaries'
  []
[]
