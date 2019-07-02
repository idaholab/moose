[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  nz = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]


[AuxVariables]
  [./v]
  [../]
[]

[AuxKernels]
  [./v]
    type = FunctionAux
    function = 'x^2+y^2'
    variable = v
    execute_on = 'initial TIMESTEP_END'
  [../]
[]

[Functions]
  [./force]
    type = ParsedFunction
    value = t
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = BodyForce
    variable = u
    function = force
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1

  solve_type = 'PJFNK'

[]

[Adaptivity]
  steps = 1
  marker = ini
  max_h_level = 2
  initial_steps = 1
  [./Indicators]
    [./err]
      type = GradientJumpIndicator
      variable = v
    [../]
  [../]
  [./Markers]
    [./ini]
      type = ErrorFractionMarker
      indicator = err
      refine = 0.7
      coarsen = 0.1
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
