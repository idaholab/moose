[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [timeu]
    type = FVTimeKernel
    variable = u
  []
  [diffu]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [forceu]
    type = FVBodyForce
    variable = u
    function = force
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 't^3*x*y'
  []
  [force]
    type = ParsedFunction
    expression = '3*x*y*t^2'
  []
[]

[FVBCs]
  [allu]
    type = FVFunctionDirichletBC
    function = exact
    variable = u
    boundary = 'left right top bottom'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  dt = 1
  end_time = 3
  scheme = 'bdf2'
[]

[Postprocessors]
  [L2u]
    type = ElementL2Error
    function = exact
    variable = u
  []
[]

[Outputs]
  csv = true
[]
