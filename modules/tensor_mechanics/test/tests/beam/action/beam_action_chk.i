# Test for checking syntax for line element action input.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0.0
  xmax = 1.0
  displacements = 'disp_x disp_y disp_z'
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0.0
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
  solve_type = PJFNK
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  dt = 1
  dtmin = 1
  end_time = 2
[]

[Modules/LineElement]
  [./block_1]
    add_variables = true

    # Material parameters
    youngs_modulus = 2.60072400269
    shear_modulus = 1.0e4
    shear_coefficient = 0.85

    # Geometry parameters
    area = 0.554256
    Iy = 0.0141889
    Iz = 0.0141889
    y_orientation = '0.0 1.0 0.0'

    eigenstrain_names = 'thermal_1'
    block = 1
  [../]
[]

[Materials]
  [./thermal_1]
    type = ComputeThermalExpansionEigenstrainBeam
    thermal_expansion_coeff = 1e-4
    temperature = 100
    stress_free_temperature = 0
    eigenstrain_name = thermal_1
    block = 1
  [../]
[]

[Postprocessors]
  [./disp_x_1]
    type = PointValue
    point = '1.0 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_x_2]
    type = PointValue
    point = '1.0 1.0 0.0'
    variable = disp_x
  [../]
[]

[Outputs]
  exodus = true
[]
