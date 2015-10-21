# Constraining slave nodes to move a linear combination of master nodes
#
# The test consists of a 2D rectangular block divided into two Quad elements
# (along its height) which have different material properties.
# A ramped displacement is applied to the top surface of the block and the
# bottom surface is fixed.
# The nodes of the interface between the two elements will tend to move as
# dictated by the material models of the two elements.

# LinearNodalConstraint forces the interface nodes to move as a linear combination
# of the nodes on the top and bottom of the block.
# Mmaster node ids and the corresponding weights are taken as input by the LinearNodalConstraint
# along with the slave node set or slave node ids.
# The constraint can be applied using either penalty or kinematic formulation.

# In this example, the final displacement of the top surface is 2m and bottom surface is 0m.
# Therefore, the final displacement of the interface nodes would be 0.25*top+0.75*bottom = 0.5m

[Mesh]
  file=rect_mid.e
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./vel_x]
  [../]
  [./accel_x]
  [../]
  [./vel_y]
  [../]
  [./accel_y]
  [../]
[]

[Kernels]
  [./DynamicTensorMechanics]
    displacements = 'disp_x disp_y'
  [../]
  [./inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
  [../]
[]

[AuxKernels]
  [./accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.5
    execute_on = timestep_end
  [../]
[]


[BCs]
  [./top_2]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 10
    function = displacement_bc_2
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
    store_stress_old = true
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
    store_stress_old = true
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
  type = Transient
  start_time = 0
  end_time = 2.0
  l_tol = 1e-8
  nl_rel_tol = 1e-8
  dt = 0.01
[]

[Constraints]
  [./disp_x_1]
    type = LinearNodalConstraint
    variable = disp_x
    master = '0 5'
    weights = '0.25 0.75'
#    slave_node_set = '2'
    slave_node_ids = '2 3'
    penalty = 1e8
    formulation = kinematic
  [../]
[]

[Functions]
  [./displacement_bc_2]
    type = PiecewiseLinear
    x = '0.0 0.5 2.0'
    y = '0.0 2.0 2.0'
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
  print_perf_log = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]
