# We run a simple problem (5 time steps and save off the solution)
# In part2, we load the solution and solve a steady problem. The test check, that the initial state in part 2 is the same as the last state from part1

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = t*((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = -4+(x*x+y*y)
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./diffusivity]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./out_diffusivity]
    type = MaterialRealAux
    variable = diffusivity
    property = diffusivity
  [../]
[]

[Kernels]
  [./ie]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = diffusivity
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Materials]
  [./mat]
    type = StatefulMaterial
    block = 0
    initial_diffusivity = 0.5
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  dt = 0.2
  start_time = 0
  num_steps = 5
[]

[Outputs]
  checkpoint = true
  [./out]
    type = Exodus
    elemental_as_nodal = true
    execute_elemental_on = none
  [../]
[]
