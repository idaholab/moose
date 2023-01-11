[Mesh]
  type = GeneratedMesh
  dim  = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx   = 4
  ny   = 4
  elem_type = QUAD9
[]

[Variables]
  active = 'u'

  [./u]
    order = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = t*t*t*((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = 3*t*t*((x*x)+(y*y))-(4*t*t*t)
  [../]
[]

[Kernels]
  active = 'diff ie ffn'

  [./ie]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
    preset = false
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  start_time = 0.0
  end_time = 1.0
  dt = 0.0625

 [./TimeIntegrator]
   type = ImplicitMidpoint
 [../]
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
[]
