# Include mfem/kernels/darcy.i
!include ../kernels/darcy.i

# A complex weak form cannot consume the real boundary conditions of a real problem.
[WeakForms]
  [Darcy]
    type = MFEMComplexWeakForm
    kernels = 'VelocityMass PressureGrad VelocityDiv'
    bcs = 'flux_boundaries'
  []
[]
