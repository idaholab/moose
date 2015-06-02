[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Problem]
#  type = ReferenceResidualProblem
#  solution_variables = 'disp_x disp_y'
#  reference_residual_variables = 'saved_x saved_y'
  XFEM_cuts ='-1.0000e-017  1.2500e+001  5.0000e+000  1.2500e+001  0.0000e+000  0.0000e+000
               5.0000e+000  1.2500e+001  2.5000e+001  1.2500e+001  0.0000e+000  1.0000e+000'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  xmin = 0.0
  xmax = 25.0
  ymin = 0.0
  ymax = 25.0
  elem_type = QUAD4
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./xfem_volfrac]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_origin_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_origin_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_origin_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_normal_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_normal_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_normal_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
#  [./saved_x]
#  [../]
#  [./saved_y]
#  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_strain_mag]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
#    save_in_disp_x = saved_x
#    save_in_disp_y = saved_y
#    xfem_volfrac = xfem_volfrac
#    use_displaced_mesh = false
  [../]
[]

[AuxKernels]
  [./xfem_volfrac]
    type = XFEMVolFracAux
    variable = xfem_volfrac
    execute_on = timestep_begin
  [../]
  [./xfem_cut_origin_x]
    type = XFEMCutPlaneAux
    variable = xfem_cut_origin_x
    quantity = origin_x
    execute_on = timestep_end
  [../]
  [./xfem_cut_origin_y]
    type = XFEMCutPlaneAux
    variable = xfem_cut_origin_y
    quantity = origin_y
    execute_on = timestep_end
  [../]
  [./xfem_cut_origin_z]
    type = XFEMCutPlaneAux
    variable = xfem_cut_origin_z
    quantity = origin_z
    execute_on = timestep_end
  [../]
  [./xfem_cut_normal_x]
    type = XFEMCutPlaneAux
    variable = xfem_cut_normal_x
    quantity = normal_x
    execute_on = timestep_end
  [../]
  [./xfem_cut_normal_y]
    type = XFEMCutPlaneAux
    variable = xfem_cut_normal_y
    quantity = normal_y
    execute_on = timestep_end
  [../]
  [./xfem_cut_normal_z]
    type = XFEMCutPlaneAux
    variable = xfem_cut_normal_z
    quantity = normal_z
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = VonMises
  [../]
  [./plastic_strain_mag]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_mag
    quantity = PlasticStrainMag
  [../]
[]

[Functions]
  [./disp_top_y]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0.1'
  [../]
[]

[BCs]
  [./top_y]
    type = FunctionPresetBC
    boundary = 2
    variable = disp_y
    function = disp_top_y
  [../]

  [./bottom_y]
    type = PresetBC
    boundary = 0
    variable = disp_y
    value = 0.0
  [../]

  [./right_x]
    type = PresetBC
    boundary = 1
    variable = disp_x
    value = 0.0
  [../]
[]

[Materials]
  [./plastic_body1]
    type = LinearStrainHardening
#    formulation = linear
    block = 0
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    yield_stress = 2e3
    hardening_constant = 1e4
    relative_tolerance = 1e-25
    absolute_tolerance = 1e-5
    disp_x = disp_x
    disp_y = disp_y
#    type = LinearIsotropicMaterial
#    block = 0
#    poissons_ratio = 0.3
#    youngs_modulus = 1e6
#    disp_x = disp_x
#    disp_y = disp_y
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
#  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
#  petsc_options_value = '201                hypre    boomeramg      8'
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'


  line_search = 'cp' #'none'

#  [./Predictor]
#    type = SimplePredictor
#    scale = 1.0
#  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-6

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-8

# time control
  start_time = 0.0
  dt = 0.02
  end_time = 1.0
  num_steps = 5000
[]


[Outputs]
  file_base = square_out
  output_initial = true
  exodus = true
#  vtk = true
#  gnuplot = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
#  [./vtk]
#    type = GNUPlot
#    output_initial = true    
#  [../]
[]
