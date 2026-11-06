!include subdomain_constraint_source.i

[WeakForms]
  [Poisson]
    type = MFEMWeakForm
    kernels = 'diff'
    constraints = 'NotAConstraint'
  []
[]
