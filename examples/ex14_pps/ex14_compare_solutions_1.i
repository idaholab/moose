[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0

  parallel_type = replicated # This uses SolutionUserObject which doesn't work with DistributedMesh.
[]

[Variables]
  [./forced]
    order = THIRD
    family = HERMITE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = forced
  [../]

  [./forcing]
    type = BodyForce
    variable = forced
    function = 'x*x+y*y' # Any object expecting a function name can also receive a ParsedFunction string
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = forced
    boundary = 'bottom right top left'
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  xda = true #XDA writes out the perfect internal state of all variables, allowing SolutionUserObject to read back in higher order solutions and adapted meshes
[]
