[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  [./forced]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  # A ParsedFunction allows us to supply analytic expressions
  # directly in the input file
  [./bc_func]
    type = ParsedFunction
    value = sin(alpha*pi*x)
    vars = alpha
    vals = 16
  [../]

  # This function is an actual compiled function
  # We could have used ParsedFunction for this as well
  [./forcing_func]
    type = ExampleFunction
    alpha = 16
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = forced
  [../]

  # This Kernel can take a function name to use
  [./forcing]
    type = BodyForce
    variable = forced
    function = forcing_func
  [../]
[]

[BCs]
  # The BC can take a function name to use
  [./all]
    type = FunctionDirichletBC
    variable = forced
    boundary = 'bottom right top left'
    function = bc_func
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  marker = uniform
  steps = 5

  # Uniformly refine the mesh
  # for the convergence study
  [./Markers]
    [./uniform]
      type = UniformMarker
      mark = REFINE
      outputs = none
    [../]
  [../]
[]

[Postprocessors]
  [./dofs]
    type = NumDOFs
  [../]
  [./integral]
    type = ElementL2Error
    variable = forced
    function = bc_func
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]
