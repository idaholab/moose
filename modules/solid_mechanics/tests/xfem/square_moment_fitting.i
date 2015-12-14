[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[XFEM]
  cut_data = '0.0000e+00   6.3330e-01   3.9000e-01   6.3330e-01   0.0000e+00   0.0000e+00
  3.9000e-01   6.3330e-01   6.8000e-01   6.3330e-01   0.0000e+00   0.0000e+00'
  qrule = moment_fitting
  output_cut_plane = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  elem_type = QUAD4
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = false
  [../]
[]

[Functions]
  [./right_trac_x]
    type = ParsedFunction
    value = '-(t*M*y)/I'
    vars = 'M E I'
    vals = '2e4 1e6 0.666666667'
  [../]
  [./bottom_disp_y]
    type = ParsedFunction
    value = '((t*M)/(2*E*I))*(1-nu*nu)*(x*x-0.25*l*l)'
    vars = 'M E I l nu'
    vals = '2e4 1e6 0.666666667 2.0 0.3'
  [../]
  [./soln_x]
    type = ParsedFunction
    value = '-(M/(E*I))*(1-nu*nu)*x*y'
    vars = 'M E I nu'
    vals = '2e4 1e6 0.666666667 0.3'
  [../]
  [./soln_y]
    type = ParsedFunction
    value = '(M/(2*E*I))*(1-nu*nu)*(x*x-0.25*l*l+(nu/(1-nu))*y*y)'
    vars = 'M E I l nu'
    vals = '2e4 1e6 0.666666667 2.0 0.3'
  [../]
[]

[BCs]
  [./right_x]
    type = FunctionNeumannBC
    boundary = 1
    variable = disp_x
    function = right_trac_x
  [../]
  [./bottom_y]
    type = FunctionPresetBC
    boundary = 0
    variable = disp_y
    function = bottom_disp_y
  [../]
  [./left_x]
    type = PresetBC
    boundary = 3
    variable = disp_x
    value = 0.0
  [../]
[]

[Materials]
  [./linelast]
    type = LinearIsotropicMaterial
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
  [./density]
    type = Density
    block = 0
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'
#  petsc_options_iname = '-ksp_gmres_restart -pc_type'
#  petsc_options_value = '201        lu'

  line_search = 'none'

#  [./Quadrature]
#    order = FOURTH
#    type = MONOMIAL
#  [../]

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 0.5
  end_time = 1.0
  num_steps = 5000
[]

[Postprocessors]
  [./numel]
    type = NumElems
    execute_on = timestep_end
  [../]
  [./integral]
    type = ElementVectorL2Error
    var_x = disp_x
    var_y = disp_y
    function_x = soln_x
    function_y = soln_y
    execute_on = timestep_end
  [../]
[]

[Outputs]
  file_base = square_moment_fitting_out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    output_linear = false
  [../]
[]
