[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[XFEM]
  geometric_cut_userobjects = 'line_seg_cut_uo'
  qrule = moment_fitting
  output_cut_plane = true
[]

[UserObjects]
  [./line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '1 0.5 10 0.5'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
  [./manager]
    type = XFEMElemPairMaterialManager
    material_names = 'material1'
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 3
  xmin = 0.0
  xmax = 10.
  ymin = 0.0
  ymax = 1
  elem_type = QUAD4
  displacements = 'disp_x disp_y'
[]

[MeshModifiers]
  [./right_bottom]
    type = AddExtraNodeset
    new_boundary = 'right_bottom'
    coord = '10.0 0.0'
  [../]
  [./right_top]
    type = AddExtraNodeset
    new_boundary = 'right_top'
    coord = '10.0 1'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./resid_y]
   # block = 0
  [../]
  [./resid_x]
  [../]
  [./traction_y]
  [../]
[]


[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    save_in_disp_y = resid_y
    save_in_disp_x = resid_x
    use_displaced_mesh = false
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    variable = stress_xx
    tensor = stress
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    variable = stress_yy
    tensor = stress
    index = 1
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
    execute_on = timestep_end
  [../]
  [./elastic_strain_yy]
    type = MaterialTensorAux
    variable = elastic_strain_yy
    tensor = elastic_strain
    index = 1
  [../]
[]

[Constraints]
  [./dispx_constraint]
    type = XFEMCohesiveConstraint
    use_displaced_mesh = false
    component = 0
    variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
    stiffness = 10
    max_traction = 0.1
    Gc = 0.0005
    manager = manager
  [../]
  [./dispy_constraint]
    type = XFEMCohesiveConstraint
    use_displaced_mesh = false
    component = 1
    variable = disp_y
    disp_x = disp_x
    disp_y = disp_y
    stiffness = 10
    max_traction = 0.1
    Gc = 0.0005
    manager = manager
  [../]
[]

#[Functions]
#  [./pull_up_and_down]
#    type = ParsedFunction
#    value = 'if(t < 8, 0.001 * t, if(t <= 16, -0.001 * (t-8) + 0.008, (t-16) * 0.001))'
#  [../]
#[]

[BCs]
  #[./bottomx]
  #  type = PresetBC
  #  boundary = right_bottom
  #  variable = disp_x
  #  value = 0.0
  #[../]
  #[./topx]
  #  type = PresetBC
  #  boundary = right_top
  #  variable = disp_x
  #  value = 0.0
  #[../]
  [./left_y]
    type = PresetBC
    boundary = left
    variable = disp_y
    value = 0.0
  [../]
  [./left_x]
    type = PresetBC
    boundary = left
    variable = disp_x
    value = 0.0
  [../]

  #[./bottomx]
  #  type = PresetBC
  #  boundary = bottom
  #  variable = disp_x
  #  value = 0.0
  #[../]
  #[./bottomy]
  #  type = PresetBC
  #  boundary = bottom
  #  variable = disp_y
  #  value = 0.0
  #[../]
  #[./topx]
  #  type = PresetBC
  #  boundary = top
  #  variable = disp_x
  #  value = 0.0
  #[../]
  [./topy]
#    type = PresetBC
    type = FunctionPresetBC
    boundary = right_top
    variable = disp_y
    #function = '0.0001*t'
    function = '0.0001 * t'
  [../]
  [./bottomy]
    type = FunctionPresetBC
    boundary = right_bottom
    variable = disp_y
    function = '-0.0001 * t'
  [../]
[]

[Materials]
  [./linelast]
    type = LinearIsotropicMaterial
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    poissons_ratio = 0.31
    youngs_modulus = 5000.
    thermal_expansion = 0.02
    t_ref = 0.5
  [../]
  [./material1]
    type = MaximumNormalSeparation
    compute = false
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

#[UserObjects]
#  [./calc_trac_x]
#    type = ComputeCohesiveTraction
#    execute_on = timestep_end
#  [../]
#  [./calc_trac_y]
#    type = ComputeCohesiveTraction
#    execute_on = timestep_end
#  [../]
#[]

[Postprocessors]
  [./react_y_top]
    type = NodalSum
    variable = resid_y
    boundary = top
  [../]
  [./react_x_top]
    type = NodalSum
    variable = resid_x
    boundary = top
  [../]
  [./react_y_bottom]
    type = NodalSum
    variable = resid_y
    boundary = bottom
  [../]
  [./react_x_bottom]
    type = NodalSum
    variable = resid_x
    boundary = bottom
  [../]
  [./react_y_left]
    type = NodalSum
    variable = resid_y
    boundary = left
  [../]
  [./react_x_left]
    type = NodalSum
    variable = resid_x
    boundary = left
  [../]
  [./disp_y]
    type = NodalMaxValue
    variable = disp_y
    boundary = top
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

  solve_type = 'PJFNK'
  #petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  #petsc_options_value = '201                hypre    boomeramg      8'


   petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
   petsc_options_value = 'lu     superlu_dist'

   line_search = bt

  #[./Predictor]
  #  type = SimplePredictor
  #  scale = 1.0
  #[../]

# controls for linear iterations
  l_max_its = 20
  l_tol = 1e-3

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5

# time control
  start_time = 0.0
  dt = 1
  end_time = 1000.0
  num_steps = 5000

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  csv = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]
