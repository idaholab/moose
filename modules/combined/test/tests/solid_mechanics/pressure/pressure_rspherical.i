#
# Prescribed pressure of 1e4 leads to xx, yy, and zz stress of 1e4.
#

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x'
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Mesh]#Comment
  file = pressure_rspherical.e
  construct_side_list_from_node_list = true
[]

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 1e4
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = SMALL
    additional_generate_output = 'stress_xx stress_yy stress_zz'
  []
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = '1'
    value = 0.0
  [../]
  [./Pressure]
    [./Pressure1]
      boundary = 2
      function = pressure
    [../]
  [../]
[]

[Materials]
  [./constant]
    type = ComputeIsotropicElasticityTensor
    block = '1 2 3'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./constant_stress]
    type = ComputeLinearElasticStress
    block = '1 2 3'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
