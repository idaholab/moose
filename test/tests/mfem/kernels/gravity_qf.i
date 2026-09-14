!include gravity.i

[Functions]
  [qf_gravity]
    type = MFEMVectorQuadratureFunction
    vector_coefficient = gravitational_force_density
    # the quadrature rule order matches the one used by VectorDomainLFIntegrator
    # for first-order elements (2 * fe_order = 2)
    order = 2
    updates = none
  []
[]

[Kernels]
  active = 'diff qf_gravity'
  [qf_gravity]
    type = MFEMVectorDomainLFKernel
    variable = displacement
    vector_coefficient = qf_gravity
  []
[]
