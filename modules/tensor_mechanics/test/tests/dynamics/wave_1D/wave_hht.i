# Wave propogation in 1D using HHT time integration
#
# The test is for an 1D bar element of length 4m  fixed on one end
# with a sinusoidal pulse dirichlet boundary condition applied to the other end.
# alpha, beta and gamma are Newmark  time integration parameters
# The equation of motion in terms of matrices is:
#
# M*accel + K*((1+alpha)*disp-alpha*disp_old) = 0
#
# Here M is the mass matrix, K is the stiffness matrix
#
# The displacement at the second, third and fourth node at t = 0.1 are
# -8.097405701570538350e-02, 2.113131879547342634e-02 and -5.182787688751439893e-03, respectively.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 4
  nz = 1
  xmin = 0.0
  xmax = 0.1
  ymin = 0.0
  ymax = 4.0
  zmin = 0.0
  zmax = 0.1
  use_displaced_mesh = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/DynamicMaster]
  [all]
    add_variables = true
    hht_alpha = -0.3
    newmark_beta = 0.3025
    newmark_gamma = 0.6
  []
[]

[BCs]
  [top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
  []
  [top_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = 0.0
  []
  [top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0.0
  []
  [right_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  []
  [right_z]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 0.0
  []
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [left_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  []
  [front_x]
    type = DirichletBC
    variable = disp_x
    boundary = front
    value = 0.0
  []
  [front_z]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0.0
  []
  [back_x]
    type = DirichletBC
    variable = disp_x
    boundary = back
    value = 0.0
  []
  [back_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [bottom_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = bottom
    function = displacement_bc
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '1 0'
  []

  [stress]
    type = ComputeLinearElasticStress
  []

  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1'
  []

[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 6.0
  l_tol = 1e-12
  nl_rel_tol = 1e-12
  dt = 0.1
[]

[Functions]
  [displacement_bc]
    type = PiecewiseLinear
    data_file = 'sine_wave.csv'
    format = columns
  []
[]

[Postprocessors]
  [_dt]
    type = TimestepSize
  []
  [disp_1]
    type = NodalVariableValue
    nodeid = 1
    variable = vel_y
  []
  [disp_2]
    type = NodalVariableValue
    nodeid = 3
    variable = vel_y
  []
  [disp_3]
    type = NodalVariableValue
    nodeid = 10
    variable = vel_y
  []
  [disp_4]
    type = NodalVariableValue
    nodeid = 14
    variable = vel_y
  []
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
