# check that the simulation terminates with an error when you try to use this
# on an element that isn't available/computed on a particular block.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names = 'prop1 prop2 prop3'
    prop_values = '1 2 42'
  [../]
[]

[VectorPostprocessors]
  [./vpp]
    type = MaterialVectorPostprocessor
    material = 'mat'
    elem_ids = '2112'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'initial timestep_end'
  csv = true
[]
