a=1.1
diff=1.1

[Mesh]
  [./gen_mesh]
    type = FileMeshGenerator
    file = skewed-0.msh
  [../]
[]

[Problem]
  kernel_coverage_check = off
[]

[Variables]
  [./v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
    type = MooseVariableFVReal
    interp_method = 'skewness-corrected'
  [../]
[]

[FVKernels]
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = ${diff}
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [exact]
    type = FVFunctionDirichletBC
    boundary = 'left right top bottom'
    function = 'exact'
    variable = v
  []
[]

[Functions]
[exact]
  type = ParsedFunction
  value = 'sin(x)*cos(y)'
[]
[forcing]
  type = ParsedFunction
  value = '2*diff*sin(x)*cos(y)'
  vars = 'a diff'
  vals = '${a} ${diff}'
[]
[]

[Executioner]
  type = Steady
  solve_type = 'FD'
  # nl_forced_its = 4
  custom_rel_tol = 1e-6
  # petsc_options_iname = '-pc_type'
  # petsc_options_value = 'hypre'
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
