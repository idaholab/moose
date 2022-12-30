[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  coord_type = 'RZ'
[]

[Variables]
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
    boundary = 'left right top bottom'
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
    expression = '1.1*sin(0.9*x)*cos(1.2*y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '1.584*sin(0.9*x)*cos(1.2*y) - (-0.891*x*sin(0.9*x)*cos(1.2*y) + 0.99*cos(0.9*x)*cos(1.2*y))/x'
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
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
