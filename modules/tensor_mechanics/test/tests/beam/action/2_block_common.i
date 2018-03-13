# Test for LineElementAction on multiple blocks

# 2 beams of length 1m are fixed at one end. Beam 1 is in block 1
# and is heated from 0 to 100 degrees. Beam 2 is in block 2 and
# is heated from 0 to 200 degrees. All the material properties
# for the two beams are identical.

# The beams have a thermal expansion coefficient of 1e-4.
# Therefore, beam 1 should expand by 0.01 m and beam 2 by 0.02m.

[Mesh]
  type = FileMesh
  file = 2_beam_block.e
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
  # parameters common to all blocks

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

  [./block_1]
    eigenstrain_names = 'thermal_1'
    block = 1
  [../]
  [./block_2]
    eigenstrain_names = 'thermal_2'
    block = 2
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
  [./thermal_2]
    type = ComputeThermalExpansionEigenstrainBeam
    thermal_expansion_coeff = 1e-4
    temperature = 200
    stress_free_temperature = 0
    eigenstrain_name = thermal_2
    block = 2
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
  file_base = '2_block_common_out'
  exodus = true
[]
