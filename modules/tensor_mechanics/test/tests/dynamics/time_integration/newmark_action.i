# Test for  Newmark time integration

# The test is for an 1D bar element of  unit length fixed on one end
# with a ramped pressure boundary condition applied to the other end.
# beta and gamma are Newmark time integration parameters
# The equation of motion in terms of matrices is:
#
# M*accel + K*disp = P*Area
#
# Here M is the mass matrix, K is the stiffness matrix, P is the applied pressure
#
# This equation is equivalent to:
#
# density*accel + Div Stress = P
#
# The first term on the left is evaluated using the Inertial force kernel
# The last term on the left is evaluated using StressDivergenceTensors
# The residual due to Pressure is evaluated using Pressure boundary condition

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmax = 0.1
  ymax = 1.0
  zmax = 0.1
  use_displaced_mesh = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/DynamicMaster]
  [all]
    add_variables = true
    newmark_beta = 0.25
    newmark_gamma = 0.5
    strain = SMALL
    density = 7750
    generate_output = 'stress_yy strain_yy'
  []
[]

[BCs]
  [top_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = 0.0
  []
  [top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
  []
  [top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0.0
  []
  [Pressure]
    [Side1]
      boundary = bottom
      function = pressure
      factor = 1
    []
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '210 0'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 2
  dt = 0.1
[]

[Functions]
  [pressure]
    type = PiecewiseLinear
    x = '0.0 0.2 1.0 5.0'
    y = '0.0 0.2 1.0 1.0'
    scale_factor = 1e3
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
  [disp]
    type = NodalExtremeValue
    variable = disp_y
    boundary = bottom
  []
  [vel]
    type = NodalExtremeValue
    variable = vel_y
    boundary = bottom
  []
  [accel]
    type = NodalExtremeValue
    variable = accel_y
    boundary = bottom
  []
  [stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  []
  [strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []

[]

[Outputs]
  exodus = true
  perf_graph = true
[]
