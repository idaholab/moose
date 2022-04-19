# test that all scalar material properties are properly recorded in basic usage.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = 'prop1 prop2 prop3'
    prop_values = '1 2 t'
  []
[]

[VectorPostprocessors]
  [vpp]
    type = MaterialVectorPostprocessor
    material = 'mat'
    elem_ids = '3 4 7 42 88'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  nl_abs_tol = 1e-12
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'initial timestep_end'
  csv = true
[]
