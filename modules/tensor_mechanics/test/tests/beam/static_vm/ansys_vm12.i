# This is a reproduction of test number 12 of ANSYS apdl verification manual.
# A 25 foot long bar is subjected to a tranverse load of 250 lb and a torsional
# moment of 9000 pb-in. The state of stress in the beam must be consistent
# with the loads applied to it.

# The radius of the bar is 2.33508 in, its area 17.129844 in, both area
# moments of inertia are I_z = I_y = 23.3505 in^4.

# A single element is used. From the external loading, the stresses are
# shear
# \tau = 9000 lb-in * radius / polar_moment = shear_modulus * theta_x/L * radius
#
# tensile stress due to bending moments
# \sigma = 250lb*300in*radius/moment_inertia = 2* radius * modulus_elast * v_{xx}

# all units inch-lb

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
    xmin = 0.0
    xmax = 300.0
  []
[]

[Modules/TensorMechanics/LineElementMaster]
  [./all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'

    # Geometry parameters
    area = 17.1298437
    Ay = 0.0
    Az = 0.0
    Iy = 23.3505405
    Iz = 23.3505405
    y_orientation = '0 1.0 0.0'
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 30.0e6
    poissons_ratio = 0.3
    shear_coefficient = 1.0
    block = 0
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 0
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 'left'
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 'left'
    value = 0.0
  [../]

  [./fixrx]
    type = DirichletBC
    variable = rot_x
    boundary = 'left'
    value = 0.0
  [../]
  [./fixry]
    type = DirichletBC
    variable = rot_y
    boundary = 'left'
    value = 0.0
  [../]
  [./fixrz]
    type = DirichletBC
    variable = rot_z
    boundary = 'left'
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_z]
    type = ConstantRate
    variable = disp_z
    boundary = 'right'
    rate = 250
  [../]

  [./force_rx]
    type = ConstantRate
    variable = rot_x
    boundary = 'right'
    rate = 9000
  [../]

[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
[Executioner]
  type = Transient
  solve_type = JFNK
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-06
  nl_abs_tol = 1e-06

  dt = 1.0
  dtmin = 0.001
  end_time = 2
[]

[Postprocessors]
  [./disp_y]
    type = PointValue
    point = '300.0 0.0 0.0'
    variable = disp_y
  [../]
  [./disp_z]
    type = PointValue
    point = '300.0 0.0 0.0'
    variable = disp_z
  [../]
  [./disp_rx]
    type = PointValue
    point = '300.0 0.0 0.0'
    variable = rot_x
  [../]
  [./disp_ry]
    type = PointValue
    point = '300.0 0.0 0.0'
    variable = rot_y
  [../]
  [./disp_rz]
    type = PointValue
    point = '300.0 0.0 0.0'
    variable = rot_z
  [../]
[]
[Debug]
 show_var_residual_norms = true
[]

[Outputs]
  csv = true
  exodus = false
[]
