[Mesh]
  file = 2d_square.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./resid_x]
  [../]
  [./resid_y]
  [../]
  [./diag_stiff_x]
  [../]
  [./diag_stiff_y]
  [../]
[]

[Functions]
  [./horizontal_movement]
    type = ParsedFunction
    value = t
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    save_in_disp_x = resid_x
    save_in_disp_y = resid_y
    diag_save_in_disp_x = diag_stiff_x
    diag_save_in_disp_y = diag_stiff_y
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./right_x]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 4
    function = horizontal_movement
  [../]
  [./right_y]
    type = PresetBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
[]

[Materials]
  [./1]
    type = LinearIsotropicMaterial
    block = 1
    disp_y = disp_y
    disp_x = disp_x
    poissons_ratio = 0.0
    youngs_modulus = 1e9
  [../]
[]

[Postprocessors]
  [./react_x]
    type = NodalSum
    variable = resid_x
    boundary = 4
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      101'
  line_search = 'none'

  nl_rel_tol = 1e-12
  l_max_its = 100
  nl_max_its = 10

  dt = 0.01
  end_time = 0.1
[]

[Outputs]
  file_base = reaction_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
