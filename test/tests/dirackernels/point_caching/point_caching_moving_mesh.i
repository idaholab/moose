[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
  uniform_refine = 4

  # Mesh is dispaced by Aux variables computed by predetermined functions
  displacements = 'disp_x disp_y'
[]

[Functions]
  [./disp_x_fn]
    type = ParsedFunction
    expression = t
  [../]

  [./disp_y_fn]
    type = ParsedFunction
    expression = 0
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./time_derivative]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./disp_x_auxk]
    type = FunctionAux
    variable = disp_x
    function = disp_x_fn
  [../]

  [./disp_y_auxk]
    type = FunctionAux
    variable = disp_y
    function = disp_y_fn
  [../]
[]

[DiracKernels]
  [./point_source]
    type = CachingPointSource
    variable = u
    # This is appropriate for this test, since we want the Dirac
    # points to be found in elements on the displaced Mesh.
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 4
  dt = .1
[]

[Outputs]
  exodus = true
[]
