# Test for eigenstrain from CSV files

# A beam of length 4 m is fixed at one end. Axial eigenstrains
# are read in from csv files. The variation of axial strains
# in the y direction and in time are mapped into the eigenstrain
# variation along axis of the beam (x) and time.

# For the first time step, the eigenstrain is 0.01 leading to 0.04m
# axial elongation. For the second time step, the eigenstrain is 0.02
# leading to 0.08 m axial elongation.

[Mesh]
  type = FileMesh
  file = beam_paper_10.e
  displacements = 'disp_x disp_y disp_z'
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = 1
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = 1
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = 1
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
#  petsc_options_iname = '-pc_type -ksp_gmres_restart'
#  petsc_options_value = 'jacobi   101'
  line_search = 'none'
#  petsc_options = '-snes_check_jacobian -snes_check_jacobian_view'
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  dt = 1
  dtmin = 1
  end_time = 2
[]

[Modules/LineElement]
  [./all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'

    # Material parameters
    youngs_modulus = 2.60072400269
    shear_modulus = 1.0e4
    shear_coefficient = 0.85

    # Geometry parameters
    area = 0.554256
    Iy = 0.0141889
    Iz = 0.0141889
    y_orientation = '0.0 1.0 0.0'

    eigenstrain_names = 'thermal'
  [../]
[]

[Materials]
  [./thermal]
    type = ComputeEigenstrainBeamFromCSVInterpolator
    disp_eigenstrain_uo = disp_uo
    to_component = 0
    position_vector = '0.0 0.0'
    eigenstrain_name = thermal
  [../]
[]

[UserObjects]
  [./disp_uo]
    type = CSVInterpolator
    csv_filename_pattern = eigenstrain_vpp_*.csv
    from_component = 1
    variable_vectors = 'axial_str'
    time_step = 1.0
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = PointValue
    point = '4.000447 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_y]
    type = PointValue
    point = '4.000447 0.0 0.0'
    variable = disp_y
  [../]
[]

[Outputs]
  file_base = 'eigenstrain_from_csv_out'
  exodus = true
[]
