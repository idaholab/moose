# Test for a mechanics problem which uses four points moment_fitting approach.
# See this paper (https://doi.org/10.1007/s00466-018-1544-2) for more details about moment_fitting approach.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[XFEM]
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
[]

[UserObjects]
  [./line_seg_cut_uo0]
    type = LineSegmentCutUserObject
    cut_data = '0.0000e+00   6.3330e-01   3.9000e-01   6.3330e-01'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
  [./line_seg_cut_uo1]
    type = LineSegmentCutUserObject
    cut_data = '3.9000e-01   6.3330e-01   6.8000e-01   6.3330e-01'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = SMALL
  [../]
[]

[Functions]
  [./right_trac_x]
    type = ParsedFunction
    expression = '-(t*M*y)/I'
    symbol_names = 'M E I'
    symbol_values = '2e4 1e6 0.666666667'
  [../]
  [./bottom_disp_y]
    type = ParsedFunction
    expression = '((t*M)/(2*E*I))*(1-nu*nu)*(x*x-0.25*l*l)'
    symbol_names = 'M E I l nu'
    symbol_values = '2e4 1e6 0.666666667 2.0 0.3'
  [../]
  [./soln_x]
    type = ParsedFunction
    expression = '-(M/(E*I))*(1-nu*nu)*x*y'
    symbol_names = 'M E I nu'
    symbol_values = '2e4 1e6 0.666666667 0.3'
  [../]
  [./soln_y]
    type = ParsedFunction
    expression = '(M/(2*E*I))*(1-nu*nu)*(x*x-0.25*l*l+(nu/(1-nu))*y*y)'
    symbol_names = 'M E I l nu'
    symbol_values = '2e4 1e6 0.666666667 2.0 0.3'
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
    type = FunctionDirichletBC
    boundary = 0
    variable = disp_y
    function = bottom_disp_y
  [../]
  [./left_x]
    type = DirichletBC
    boundary = 3
    variable = disp_x
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

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
  nl_rel_tol = 1e-16
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 0.5
  end_time = 1.0
  num_steps = 5000
[]

[Postprocessors]
  [./numel]
    type = NumElements
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
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
