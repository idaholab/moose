[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
[]

[Variables]
  [u]
  []
  [u_adjoint]
    nl_sys = adjoint
  []
[]

[Kernels]
  [diff]
    type = ADMatDiffusion
    variable = u
    diffusivity = D
  []
  [src]
    type = ADBodyForce
    variable = u
    value = 1
  []

  [src_adjoint]
    type = ADBodyForce
    variable = u_adjoint
    value = 1
  []
[]

[BCs]
  [dirichlet]
    type = ADDirichletBC
    variable = u
    boundary = 'top right'
    value = 0
  []
[]

[Materials]
  [diffc]
    type = ADParsedMaterial
    property_name = D
    expression = '0.1 + 5 * u'
    coupled_variables = 'u'
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
  [u_adjoint_avg]
    type = ElementAverageValue
    variable = u_adjoint
    execute_on = ADJOINT_TIMESTEP_END
  []
  [inner_product]
    type = VariableInnerProduct
    variable = u
    second_variable = u_adjoint
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint

  nl_rel_tol = 1e-12
  l_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
