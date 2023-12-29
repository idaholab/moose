# Test for  Acceleration boundary condition

# This test contains one brick element which is fixed in the y and z direction.
# Base acceleration is applied in the x direction to all nodes on the bottom surface (y=0).

# The PresetAcceleration converts the given acceleration to a displacement
# using Newmark time integration. This displacement is then prescribed on the boundary.
#
# Result: The acceleration at the bottom node should be same as the input acceleration
# which is a triangular function with peak at t = 0.2 in this case. Width of the triangular function
# is 0.2 s.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0.0
  xmax = 0.1
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 0.1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
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
  [./vel_z]
  [../]
  [./accel_z]
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]

[Kernels]
  [./TensorMechanics]
  [../]
  [./inertia_x]
    type = InertialForce
    variable = disp_x
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
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
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 0
    index_j = 1
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 0
    index_j = 1
  [../]

[]

[Functions]
  [./acceleration_bottom]
    type = PiecewiseLinear
    data_file = acceleration.csv
    format = columns
  [../]
[]

[BCs]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value=0.0
  [../]
  [./top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value=0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value=0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value=0.0
  [../]
  [./preset_accelertion]
    type = PresetAcceleration
    boundary = bottom
    function = acceleration_bottom
    variable = disp_x
    beta = 0.25
    acceleration = accel_x
    velocity = vel_x
   [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '210e9 0'
  [../]

  [./strain]
    type = ComputeSmallStrain
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '7750'
  [../]

[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      101'
  start_time = 0
  end_time = 2.0
  dt = 0.01
  dtmin = 0.01
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_tol = 1e-8
  timestep_tolerance = 1e-8

  # Time integrator scheme
  schem = "newmark-beta"
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
  [./disp]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 1
  [../]
  [./vel]
    type = NodalVariableValue
    variable = vel_x
    nodeid = 1
  [../]
  [./accel]
    type = NodalVariableValue
    variable = accel_x
    nodeid = 1
  [../]
[]

[Outputs]
  file_base = "AccelerationBC_test_out"
  exodus = true
  perf_graph = true
[]
