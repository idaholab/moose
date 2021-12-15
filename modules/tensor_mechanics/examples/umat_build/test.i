[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
  []
[]

[Materials]
  [umat]
    type = AbaqusUMATStress
    constant_properties = '1000 0.3'
    num_state_vars = 0
    plugin = umat
    use_one_based_indexing = true
  []
[]

[Executioner]
  type = Steady
[]
