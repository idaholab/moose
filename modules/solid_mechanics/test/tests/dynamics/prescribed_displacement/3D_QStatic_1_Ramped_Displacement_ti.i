# One 3D element under ramped displacement loading.
#
# loading:
# time : 0.0 0.1  0.2  0.3
# disp : 0.0 0.0 -0.01 -0.01

# This displacement loading is applied using the PresetDisplacement boundary condition.

# Here, the given displacement time history is converted to an acceleration
# time history using Backward Euler time differentiation. Then, the resulting
# acceleration is integrated using Newmark time integration to obtain a
# displacement time history which is then applied to the boundary.

# This is done because if the displacement is applied using Dirichlet BC, the
# resulting acceleration is very noisy.

# Boundaries:
# x = 0 left
# x = 1 right
# y = 0 bottom
# y = 1 top
# z = 0 back
# z = 1 front

# Result: The displacement at the top node in the z direction should match
# the prescribed displacement. Also, the z acceleration should
# be two triangular pulses, one peaking at 0.1 and another peaking at
# 0.2.


[Mesh]
  type = GeneratedMesh
  dim = 3 # Dimension of the mesh
  nx = 1 # Number of elements in the x direction
  ny = 1 # Number of elements in the y direction
  nz = 1 # Number of elements in the z direction
  xmin = 0.0
  xmax = 1
  ymin = 0.0
  ymax = 1
  zmin = 0.0
  zmax = 1
  allow_renumbering = false # So NodalVariableValue can index by id
[]

[Variables] # variables that are solved
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables] # variables that are calculated for output
  [./accel_x]
  [../]
  [./vel_x]
  [../]
  [./accel_y]
  [../]
  [./vel_y]
  [../]
  [./accel_z]
  [../]
  [./vel_z]
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./DynamicTensorMechanics] # zeta*K*vel + K * disp
    displacements = 'disp_x disp_y disp_z'
    stiffness_damping_coefficient = 0.000025
  [../]
  [./inertia_x] # M*accel + eta*M*vel
    type = InertialForce
    variable = disp_x
    eta = 19.63
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    eta = 19.63
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    eta = 19.63
  [../]
[]

[AuxKernels]
  [./accel_x] # These auxkernels are only to check output
    type = TestNewmarkTI
    displacement = disp_x
    variable = accel_x
    first = false
  [../]
  [./accel_y]
    type = TestNewmarkTI
    displacement = disp_y
    variable = accel_y
    first = false
  [../]
  [./accel_z]
    type = TestNewmarkTI
    displacement = disp_z
    variable = accel_z
    first = false
  [../]
  [./vel_x]
    type = TestNewmarkTI
    displacement = disp_x
    variable = vel_x
  [../]
  [./vel_y]
    type = TestNewmarkTI
    displacement = disp_y
    variable = vel_y
  [../]
  [./vel_z]
    type = TestNewmarkTI
    displacement = disp_z
    variable = vel_z
  [../]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
  [../]
[]

[Functions]
  [./displacement_front]
    type = PiecewiseLinear
    data_file = 'displacement.csv'
    format = columns
  [../]
[]

[BCs]
  [./Preset_displacement]
    type = PresetDisplacement
    variable = disp_z
    function = displacement_front
    boundary = front
    beta = 0.25
    velocity = vel_z
    acceleration = accel_z
  [../]
  [./anchor_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./anchor_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./anchor_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    youngs_modulus = 325e6 #Pa
    poissons_ratio = 0.3
    type = ComputeIsotropicElasticityTensor
    block = 0
  [../]
  [./strain]
    #Computes the strain, assuming small strains
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    #Computes the stress, using linear elasticity
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = density
    prop_values = 2000 #kg/m3
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 3.0
  l_tol = 1e-6
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  dt = 0.1
  timestep_tolerance = 1e-6

  # Time integrator scheme
  scheme = "newmark-beta"
[]

[Postprocessors] # These quantites are printed to a csv file at every time step
  [./_dt]
    type = TimestepSize
  [../]
  [./accel_6x]
    type = NodalVariableValue
    nodeid = 6
    variable = accel_x
  [../]
  [./accel_6y]
    type = NodalVariableValue
    nodeid = 6
    variable = accel_y
  [../]
  [./accel_6z]
    type = NodalVariableValue
    nodeid = 6
    variable = accel_z
  [../]
  [./vel_6x]
    type = NodalVariableValue
    nodeid = 6
    variable = vel_x
  [../]
  [./vel_6y]
    type = NodalVariableValue
    nodeid = 6
    variable = vel_y
  [../]
  [./vel_6z]
    type = NodalVariableValue
    nodeid = 6
    variable = vel_z
  [../]
  [./disp_6x]
    type = NodalVariableValue
    nodeid = 6
    variable = disp_x
  [../]
  [./disp_6y]
    type = NodalVariableValue
    nodeid = 6
    variable = disp_y
  [../]
  [./disp_6z]
    type = NodalVariableValue
    nodeid = 6
    variable = disp_z
  [../]
[]

[Outputs]
  file_base = "3D_QStatic_1_Ramped_Displacement_out"
  exodus = true
  perf_graph = true
[]
