# Checking that the InertialTorque calculates the correct residual.
# This input file does not have any physical meaning!  It is simply checking
# the residual is computed correctly in a very simple setting.
#
# The following displacements are prescribed
# disp_x = 1+t
# disp_y = -2(1+t)
# disp_z = 2(1+t)
# along with the velocities (which don't follow from the displacements!)
# vel_x = -2(t+1)
# vel_y = -5(t+1)
# vel_z = t+1
# and accelerations
# accel_x = -t+2
# accel_y = -5t+2
# accel_z = t+2
#
# Using the Newmark + Damping parameters
# beta = 1/4
# gamma = 1/2
# eta = 1/4
# alpha = 1/2
# There give
# accel_x = 11.75
# accel_y = 11
# accel_z = 3
#
# The InertialTorque should compute
# Residual_0 = rho * eps_0jk * disp_j * accel_k
#            = rho * (disp_y * accel_z - disp_z * accel_y)
#            = -56 * rho
# Residual_1 = rho * eps_0jk * disp_j * accel_k
#            = rho * (disp_z * accel_x - disp_x * accel_z)
#            = 41 * rho
# Residual_2 = rho * eps_2jk * disp_j * accel_k
#            = rho * (disp_x * accel_y - disp_y * accel_x)
#            = 69 * rho
# These get integrated over the unit element to give (1/8)^th of these
# values at each node
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  velocities = 'vel_x vel_y vel_z'
  accelerations = 'accel_x accel_y accel_z'
  beta = 0.25
  gamma = 0.5
  alpha = 0.5
  eta = 0.25
[]

[Variables]
  [./wc_x]
  [../]
  [./wc_y]
  [../]
  [./wc_z]
  [../]
[]

[Kernels]
  [./icm_x]
    type = InertialTorque
    component = 0
    variable = wc_x
    save_in = res_x
  [../]
  [./icm_y]
    type = InertialTorque
    component = 1
    variable = wc_y
    density = another_density
    save_in = res_y
  [../]
  [./icm_z]
    type = InertialTorque
    component = 2
    variable = wc_z
    density = yet_another_density
    save_in = res_z
  [../]
[]

[AuxVariables]
  [./res_x]
  [../]
  [./res_y]
  [../]
  [./res_z]
  [../]
  [./disp_x]
    initial_condition = 1
  [../]
  [./disp_y]
    initial_condition = -2
  [../]
  [./disp_z]
    initial_condition = 2
  [../]
  [./vel_x]
    initial_condition = -2
  [../]
  [./vel_y]
    initial_condition = -5
  [../]
  [./vel_z]
    initial_condition = 1
  [../]
  [./accel_x]
    initial_condition = 2
  [../]
  [./accel_y]
    initial_condition = 2
  [../]
  [./accel_z]
    initial_condition = 2
  [../]
[]

[AuxKernels]
  [./disp_x]
    type = FunctionAux
    variable = disp_x
    function = '1+t'
  [../]
  [./disp_y]
    type = FunctionAux
    variable = disp_y
    function = '-2*(1+t)'
  [../]
  [./disp_z]
    type = FunctionAux
    variable = disp_z
    function = '2*(1+t)'
  [../]
  [./vel_x]
    type = FunctionAux
    variable = vel_x
    function = '-2*t'
  [../]
  [./vel_y]
    type = FunctionAux
    variable = vel_y
    function = '-5*t'
  [../]
  [./vel_z]
    type = FunctionAux
    variable = vel_z
    function = 't'
  [../]
  [./accel_x]
    type = FunctionAux
    variable = accel_x
    function = '-t+2'
  [../]
  [./accel_y]
    type = FunctionAux
    variable = accel_y
    function = '-5*t+2'
  [../]
  [./accel_z]
    type = FunctionAux
    variable = accel_z
    function = 't+2'
  [../]
[]

[Postprocessors]
  [./res_x]
    type = PointValue
    point = '0 0 0'
    variable = res_x
  [../]
  [./res_y]
    type = PointValue
    point = '0 0 0'
    variable = res_y
  [../]
  [./res_z]
    type = PointValue
    point = '0 0 0'
    variable = res_z
  [../]
[]

[Materials]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'density another_density yet_another_density'
    prop_values = '2.0 8.0 16.0'
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  nl_abs_tol = 1E30 # large because there is no way of getting to residual=0 here
[]

[Outputs]
  csv = true
[]
