[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 10
    ny = 10
    dim = 2
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
    input = gen
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    input = block1
  []
  [breakmesh]
    input = block2
    type = BreakMeshByBlockGenerator
    block_pairs = '1 2'
    split_interface = true
    add_interface_on_two_sides = true
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [thermal_cond]
    type = HeatConduction
    variable = temperature
  []
[]

[InterfaceKernels]
  [thin_layer]
    type = ThinLayerHeatTransfer
    thermal_conductivity = thermal_conductivity_layer
    thickness = 0.01
    variable = temperature
    neighbor_var = temperature
    boundary = Block1_Block2
  []
[]

[BCs]
  [left_temp]
    type = DirichletBC
    value = 100
    variable = temperature
    boundary = left
  []
  [right_temp]
    type = DirichletBC
    value = 0
    variable = temperature
    boundary = right
  []
[]

[Materials]
  [thermal_cond]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1'
  []
  [thermal_cond_layer]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity_layer'
    prop_values = '0.05'
    boundary = Block1_Block2
  []
[]
[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  dt = 0.05
  num_steps = 1
[]


[Outputs]
  print_linear_residuals = false
  exodus = true
[]
