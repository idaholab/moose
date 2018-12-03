# Test for rayleigh damping implemented using Newmark time integration

# The test is for an 1-D bar element of unit length fixed on one end
# with a ramped pressure boundary condition applied to the other end.
# zeta and eta correspond to the stiffness and mass proportional
# rayleigh damping beta and gamma are Newmark time integration
# parameters The equation of motion in terms of matrices is:
#
# M*accel + eta*M*vel + zeta*K*vel + K*disp = P*Area
#
# Here M is the mass matrix, K is the stiffness matrix, P is the applied pressure
#
# This equation is equivalent to:
#
# density*accel + eta*density*vel + zeta*d/dt(Div stress) + Div stress = P
#
# The first two terms on the left are evaluated using the Inertial
# force kernel The next two terms on the left involving zeta ise
# evaluated using the StressDivergence Kernel The residual due to
# Pressure is evaluated using Pressure boundary condition
#
# The system will come to steady state slowly after the pressure
# becomes constant.
[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

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
  [./inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
    eta=0.1
  [../]
  [./stiffness_x]
    type = StressDivergence
    variable = disp_x
    component = 0
    zeta = 0.1
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
    eta=0.1
  [../]
  [./stiffness_y]
    type = StressDivergence
    variable = disp_y
    component = 1
    zeta = 0.1
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    beta = 0.25
    gamma = 0.5
    eta = 0.1
  [../]
  [./stiffness_z]
    type = StressDivergence
    variable = disp_z
    component = 2
    zeta = 0.1
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
  [./accel_z]
    type = NewmarkAccelAux
    variable = accel_z
    displacement = disp_z
    velocity = vel_z
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_z]
    type = NewmarkVelAux
    variable = vel_z
    acceleration = accel_z
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./stress_yy]
     type = MaterialTensorAux
     variable = stress_yy
     tensor = stress
     index = 1
  [../]
  [./strain_yy]
     type = MaterialTensorAux
     variable = strain_yy
     tensor = total_strain
     index = 1
  [../]

[]


[BCs]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value=0.0
  [../]
  [./top_x]
   type = DirichletBC
    variable = disp_x
    boundary = top
    value=0.0
  [../]
  [./top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value=0.0
  [../]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value=0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value=0.0
  [../]
  [./Pressure]
    [./Side1]
    boundary = bottom
    function = pressure
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    factor = 1
    [../]
  [../]
[]

[Materials]

  [./constant]
    type = Elastic
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 210e+09
    poissons_ratio = 0
    thermal_expansion = 0
  [../]

  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'density'
    prop_values = '7750'
  [../]

[]

[Executioner]

  type = Transient
  start_time = 0
  end_time = 2
  dtmax = 0.1
  dtmin = 0.1
  [./TimeStepper]
    type = ConstantDT
    dt = 0.1
  [../]

[]


[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2 1.0 2.0 5.0'
    y = '0.0 0.1 0.2 1.0 1.0 1.0'
    scale_factor = 1e9
  [../]
  [./vel_ic]
    type = PiecewiseLinear
    x = '0.0 0.5 1.0'
    y = '0.1 0.1 0.1'
    scale_factor = 1
  [../]
[]

[Postprocessors]
   [./_dt]
     type = TimestepSize
   [../]
   [./disp]
     type = NodalMaxValue
     variable = disp_y
     boundary = bottom
   [../]
   [./vel]
     type = NodalMaxValue
     variable = vel_y
     boundary = bottom
   [../]
   [./accel]
     type = NodalMaxValue
     variable = accel_y
     boundary = bottom
   [../]
   [./stress_yy]
      type = ElementAverageValue
      variable = stress_yy
   [../]
   [./strain_yy]
      type = ElementAverageValue
      variable = strain_yy
   [../]

[]

[Outputs]
  exodus = true
[]
