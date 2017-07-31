# This is a test of the usage of xfem with contact. The body on
# the slave side is split with XFEM and pushed into an uncracked
# body.

[Mesh]
  file = 2blocks.e
  displacements = 'disp_x disp_y'
  patch_size = 80
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./penetration]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./negramp]
    type = ParsedFunction
    value = -t/10
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[AuxKernels]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 3
    paired_boundary = 2
  [../]
[]

[Postprocessors]
  [./nonlinear_its]
    type = NumNonlinearIterations
    execute_on = timestep_end
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
    function = negramp
  [../]
  [./right_y]
    type = PresetBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
[]

[Materials]
  [./left]
    type = LinearIsotropicMaterial
    block = '1 2'
    disp_y = disp_y
    disp_x = disp_x
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
[]

[Contact]
  [./leftright]
    slave = 3
    disp_y = disp_y
    disp_x = disp_x
    master = 2
    model = frictionless
    penalty = 1e+6
    formulation = default
    system = constraint
    normal_smoothing_distance = 0.1
  [../]
[]

[XFEM]
  cut_data = '0.51 0.0 1.01 0.0 0.0 0.0'
  qrule = volfrac
  output_cut_plane = true
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 1000
  dt = 0.2
  end_time = 2.0
  l_tol = 1e-6
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-9
[]

[Outputs]
  exodus = true
  console = true
[]
