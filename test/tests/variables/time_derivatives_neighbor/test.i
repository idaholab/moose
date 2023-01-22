[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 4
  nx = 2
[]

[Functions]
  [a_fn]
    type = ParsedFunction
    expression = 't*(t+x)'
  []
[]

[AuxVariables]
  [a]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [a_ak]
    type = FunctionAux
    variable = a
    function = a_fn
  []
[]

[Materials]
  [cm]
    type = CoupledValuesMaterial
    variable = a
  []
[]

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = u
  []
[]

[DGKernels]
  [dgk]
    type = MatDGKernel
    variable = u
    mat_prop = a_value
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3

  [TimeIntegrator]
    type = NewmarkBeta
  []
  [Quadrature]
    type = GAUSS
    order = FIRST
  []
[]

[Outputs]
  [./out]
    type = Exodus
    output_material_properties = true
    show_material_properties = 'a_value a_dot a_dot_dot a_dot_du a_dot_dot_du'
    execute_on = 'TIMESTEP_END'
  [../]
[]
