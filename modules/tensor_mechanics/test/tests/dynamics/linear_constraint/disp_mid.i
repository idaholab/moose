# Constraining secondary nodes to move a linear combination of primary nodes
#
# The test consists of a 2D rectangular block divided into two Quad elements
# (along its height) which have different material properties.
# A displacement of 2 m is applied to the top surface of the block in x direction and the
# bottom surface is held fixed.
# The nodes of the interface between the two elements will tend to move as
# dictated by the material models of the two elements.

# LinearNodalConstraint forces the interface nodes to move as a linear combination
# of the nodes on the top and bottom of the block.
# primary node ids and the corresponding weights are taken as input by the LinearNodalConstraint
# along with the secondary node set or secondary node ids.
# The constraint can be applied using either penalty or kinematic formulation.

# In this example, the final x displacement of the top surface is 2m and bottom surface is 0m.
# Therefore, the final x displacement of the interface nodes would be 0.25*top+0.75*bottom = 0.5m

[Mesh]
  file=rect_mid.e
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
  [../]
[]

[BCs]
  [./top_2x]
    type = DirichletBC
    variable = disp_x
    boundary = 10
    value = 2.0
  [../]
  [./top_2y]
    type = DirichletBC
    variable = disp_y
    boundary = 10
    value = 0.0
  [../]
  [./bottom_1]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./bottom_2]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
[]

[Materials]
  [./Elasticity_tensor_1]
    type = ComputeElasticityTensor
    block = 1
    fill_method = 'symmetric_isotropic'
    C_ijkl = '400. 200.'
  [../]

  [./strain_1]
    type = ComputeSmallStrain
    block = 1
    displacements = 'disp_x disp_y'
  [../]

  [./stress_1]
    type = ComputeLinearElasticStress
    block = 1
  [../]

  [./density_1]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'density'
    prop_values = '10.'
  [../]

  [./Elasticity_tensor_2]
    type = ComputeElasticityTensor
    block = 2
    fill_method = 'symmetric_isotropic'
    C_ijkl = '1000. 500.'
  [../]

  [./strain_2]
    type = ComputeSmallStrain
    block = 2
    displacements = 'disp_x disp_y'
  [../]

  [./stress_2]
    type = ComputeLinearElasticStress
    block = 2
  [../]

  [./density_2]
    type = GenericConstantMaterial
    block = 2
    prop_names = 'density'
    prop_values = '10.'
  [../]

[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = ''
  petsc_options_value = ''
  line_search = 'none'
[]

[Constraints]
  [./disp_x_1]
    type = LinearNodalConstraint
    variable = disp_x
    primary = '0 5'
    weights = '0.25 0.75'
#    secondary_node_set = '2'
    secondary_node_ids = '2 3'
    penalty = 1e8
    formulation = kinematic
  [../]
  [./disp_y_1]
    type = LinearNodalConstraint
    variable = disp_y
    primary = '0 5'
    weights = '0.25 0.75'
#    secondary_node_set = '2'
    secondary_node_ids = '2 3'
    penalty = 1e8
    formulation = kinematic
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
  [./disp_1]
    type = NodalVariableValue
    nodeid = 0
    variable = disp_x
  [../]
  [./disp_2]
    type = NodalVariableValue
    nodeid = 1
    variable = disp_x
  [../]
  [./disp_3]
    type = NodalVariableValue
    nodeid = 2
    variable = disp_x
  [../]
  [./disp_4]
    type = NodalVariableValue
    nodeid = 3
    variable = disp_x
  [../]
  [./disp_5]
    type = NodalVariableValue
    nodeid = 4
    variable = disp_x
  [../]
  [./disp_6]
    type = NodalVariableValue
    nodeid = 5
    variable = disp_x
  [../]
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
  perf_graph = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
