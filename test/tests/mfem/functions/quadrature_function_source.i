!include parsed_function_source.i

[Functions]
  [qf_source]
    type = MFEMScalarQuadratureFunction
    coefficient = source
    # match the default integration rule order used by DomainLFIntegrator
    # for first-order elements (oa * fe_order + ob = 2 * 1 + 0)
    order = 2
  []
[]

[Kernels]
  active = 'diff qf_source'
  [qf_source]
    type = MFEMDomainLFKernel
    variable = variable
    coefficient = qf_source
    block = wire
  []
[]
