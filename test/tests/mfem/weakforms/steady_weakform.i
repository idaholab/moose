# Include mfem/kernels/darcy.i
!include ../kernels/darcy.i

[WeakForms]
  [Darcy]
    type = MFEMWeakForm
    kernels = 'VelocityMass PressureGrad VelocityDiv'
    bcs = 'flux_boundaries'
  []
[]
