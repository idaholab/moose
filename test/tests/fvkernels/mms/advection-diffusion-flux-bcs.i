diff=1.1
a=1.1

[GlobalParams]
  advected_interp_method = 'average'
[]

[Mesh]
  [./gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = -0.6
    xmax = 0.6
    nx = 2
  [../]
[]

[Problem]
  kernel_coverage_check = off
  fv_bcs_integrity_check = false
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
  [advection]
    type = FVAdvectionFunctionBC
    boundary = 'left right'
    variable = v
    exact_solution = 'exact'
    velocity = '${a} 0 0'
  []
  [diffusion]
    type = FVDiffusionFunctionBC
    boundary = 'left right'
    variable = v
    exact_solution = 'exact'
    coeff = coeff
    coeff_function = '${diff}'
  []
  [exact]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    function = 'exact'
    variable = v
  []
[]

[Materials]
  [diff]
    type = ADGenericConstantMaterial
    prop_names = 'coeff'
    prop_values = '${diff}'
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    value = '1.1 * sin(1.1 * x)'
  []
  [forcing]
    type = ParsedFunction
    value = '${diff} * 1.1 * 1.1 * 1.1 * sin(1.1 * x) + ${a} * 1.1 * 1.1 * cos(1.1 * x)'
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
