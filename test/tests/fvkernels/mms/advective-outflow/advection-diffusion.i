diff=1
a=1

[GlobalParams]
  advected_interp_method = 'average'
[]

[Mesh]
  [./gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = -1
    xmax = 0
    nx = 2
  [../]
[]

[Variables]
  [./v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]
[]

[FVKernels]
  [./advection]
    type = FVAdvection
    variable = v
    velocity = '${a} 0 0'
    force_boundary_execution = true
  [../]
  [./diffusion]
    type = FVDiffusion
    variable = v
    coeff = coeff
  [../]
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [left]
    type = FVFunctionDirichletBC
    boundary = 'left'
    function = 'exact'
    variable = v
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '${diff}'
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'cos(x)'
  []
  [forcing]
    type = ParsedFunction
    expression = 'cos(x) - sin(x)'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
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
