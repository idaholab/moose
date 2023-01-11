[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  # [u]
  # []
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FVKernels]
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [boundary]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    function = 'exact'
    variable = v
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = '3*x^2 + 2*x + 1'
  []
  [forcing]
    type = ParsedFunction
    expression = '-6'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  # [./L2u]
  #   type = ElementL2Error
  #   variable = u
  #   function = exact
  #   outputs = 'console'
  #   execute_on = 'timestep_end'
  # [../]
  [./error]
    type = ElementL2Error
    variable = v
    function = exact
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
