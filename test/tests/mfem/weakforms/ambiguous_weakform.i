# Include mfem/kernels/darcy.i
!include ../kernels/darcy.i

# Two weak forms, but nothing names which one the default problem composer should use.
[WeakForms]
  [Darcy]
    type = MFEMWeakForm
    kernels = 'VelocityMass PressureGrad VelocityDiv'
    bcs = 'flux_boundaries'
  []
  [VelocityOnly]
    type = MFEMWeakForm
    kernels = 'VelocityMass'
    bcs = 'flux_boundaries'
  []
[]
