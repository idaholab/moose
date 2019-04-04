[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmax = 2
  ny = 2
  ymax = 2
  elem_type = QUAD4
[]

[MeshModifiers]
  [./subdomain_id]
    type = SubdomainBoundingBox
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]

  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain_id
    master_block = '0'
    paired_block = '1'
    new_boundary = 'interface'
  [../]

[]

[Functions]
  [./fn_exact]
    type = ParsedFunction
    value = 'x*x+y*y'
  [../]

  [./ffn]
    type = ParsedFunction
    value = -4
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]


[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = ffn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = fn_exact
  [../]
[]

[Materials]
  [./stateful1]
    type = StatefulMaterial
    block = 0
    initial_diffusivity = 5
    # outputs = all
  [../]
  [./stateful2]
    type = StatefulMaterial
    block = 1
    initial_diffusivity = 2
    # outputs = all
  [../]
  [./interface_material_avg]
      type = InterfaceValueMaterial
      mat_prop_master = diffusivity
      mat_prop_slave = diffusivity
      mat_prop_out_basename = diff
      boundary = interface
      interface_value_type = average
      var_master = diffusivity_1
      var_slave = diffusivity_2
      mat_prop_var_out_basename = diff_var
  [../]
  [./interface_material_jump_master_minus_slave]
      type = InterfaceValueMaterial
      mat_prop_master = diffusivity
      mat_prop_slave = diffusivity
      mat_prop_out_basename = diff
      boundary = interface
      interface_value_type = jump_master_minus_slave
      var_master = diffusivity_1
      var_slave = diffusivity_2
      mat_prop_var_out_basename = diff_var
  [../]
  [./interface_material_jump_slave_minus_master]
      type = InterfaceValueMaterial
      mat_prop_master = diffusivity
      mat_prop_slave = diffusivity
      mat_prop_out_basename = diff
      boundary = interface
      interface_value_type = jump_slave_minus_master
      var_master = diffusivity_1
      var_slave = diffusivity_2
      mat_prop_var_out_basename = diff_var
  [../]
  [./interface_material_jump_abs]
      type = InterfaceValueMaterial
      mat_prop_master = diffusivity
      mat_prop_slave = diffusivity
      mat_prop_out_basename = diff
      boundary = interface
      interface_value_type = jump_abs
      var_master = diffusivity_1
      var_slave = diffusivity_2
      mat_prop_var_out_basename = diff_var
  [../]
  [./interface_material_master]
      type = InterfaceValueMaterial
      mat_prop_master = diffusivity
      mat_prop_slave = diffusivity
      mat_prop_out_basename = diff
      boundary = interface
      interface_value_type = master
      var_master = diffusivity_1
      var_slave = diffusivity_2
      mat_prop_var_out_basename = diff_var
  [../]
  [./interface_material_slave]
      type = InterfaceValueMaterial
      mat_prop_master = diffusivity
      mat_prop_slave = diffusivity
      mat_prop_out_basename = diff
      var_master = diffusivity_1
      var_slave = diffusivity_2
      mat_prop_var_out_basename = diff_var
      boundary = interface
      interface_value_type = slave
  [../]
[]

[AuxKernels]
  [./diffusivity_1]
    type = MaterialRealAux
    property = diffusivity
    variable = diffusivity_1
    block = 0
  []
  [./diffusivity_2]
    type = MaterialRealAux
    property = diffusivity
    variable = diffusivity_2
    block = 1
  []
  [./interface_material_avg]
    type = MaterialRealAux
    property = diff_average
    variable = diffusivity_average
    boundary = interface
  []
  [./interface_material_jump_master_minus_slave]
    type = MaterialRealAux
    property = diff_jump_master_minus_slave
    variable = diffusivity_jump_master_minus_slave
    boundary = interface
  []
  [./interface_material_jump_slave_minus_master]
    type = MaterialRealAux
    property = diff_jump_slave_minus_master
    variable = diffusivity_jump_slave_minus_master
    boundary = interface
  []
  [./interface_material_jump_abs]
    type = MaterialRealAux
    property = diff_jump_abs
    variable = diffusivity_jump_abs
    boundary = interface
  []
  [./interface_material_master]
    type = MaterialRealAux
    property = diff_master
    variable = diffusivity_master
    boundary = interface
  []
  [./interface_material_slave]
    type = MaterialRealAux
    property = diff_slave
    variable = diffusivity_slave
    boundary = interface
  []


[]

[AuxVariables]
  [./diffusivity_1]
    family = MONOMIAL
    order = CONSTANT
  []
  [./diffusivity_2]
    family = MONOMIAL
    order = CONSTANT
  []
  [./diffusivity_average]
    family = MONOMIAL
    order = CONSTANT
  []
  [./diffusivity_jump_master_minus_slave]
    family = MONOMIAL
    order = CONSTANT
  []
  [./diffusivity_jump_slave_minus_master]
    family = MONOMIAL
    order = CONSTANT
  []
  [./diffusivity_jump_abs]
    family = MONOMIAL
    order = CONSTANT
  []
  [./diffusivity_master]
    family = MONOMIAL
    order = CONSTANT
  []
  [./diffusivity_slave]
    family = MONOMIAL
    order = CONSTANT
  []
[]


[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
