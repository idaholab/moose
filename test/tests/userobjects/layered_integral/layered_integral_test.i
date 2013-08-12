[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  nz = 6
  # This test currently diffs when run in parallel with ParallelMesh enabled,
  # most likely due to the fact that CONSTANT MONOMIALS are currently not written
  # out correctly in this case.  For more information, see #2122.
  distribution = serial
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./layered_integral]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./y]
    type = ParsedFunction
    value = y
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./liaux]
    type = SpatialUserObjectAux
    variable = layered_integral
    execute_on = timestep
    user_object = layered_integral
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  [../]
[]

[UserObjects]
  [./layered_integral]
    type = LayeredIntegral
    direction = y
    num_layers = 3
    variable = u
    execute_on = residual
  [../]
[]

[Executioner]
  type = Steady
[]

[Output]
  file_base = out
  output_initial = true
  exodus = true
[]

