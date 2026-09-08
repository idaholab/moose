# Include mfem/kernels/darcy.i
!include ../kernels/darcy.i

# Two weak forms are present, so the problem composer and the solver must each name the one
# whose equation system they act on.
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

[ProblemComposers]
  [Darcy]
    type = MFEMWeakFormProblemComposer
    weak_form = Darcy
  []
[]

[Solvers]
  [main]
    weak_form = Darcy
  []
[]
