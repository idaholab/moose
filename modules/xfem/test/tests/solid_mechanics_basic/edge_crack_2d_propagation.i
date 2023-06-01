[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
[block]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 5
  ny = 5
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  elem_type = QUAD4
[]
[]

[UserObjects]
  [./cut_mesh]
    type = MeshCut2DFunctionUserObject
    mesh_file = 2D_edge_crack.e
    growth_direction_x = growth_func_x
    growth_direction_y = growth_func_y
    growth_rate = growth_func_v
  [../]
[]

[Functions]
  [./growth_func_x]
    type = ParsedFunction
    expression = 0.4*t
  [../]
  [./growth_func_y]
    type = ParsedFunction
    expression = 1.8*(t-1)
  [../]
  [./growth_func_v]
    type = ParsedFunction
    expression = 0.1*t
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    planar_formulation = plane_strain
    add_variables = true
    generate_output = 'stress_xx stress_yy vonmises_stress'
  [../]
[]

[Functions]
  [./top_trac_y]
    type = ConstantFunction
    value = 10
  [../]
[]


[BCs]
  [./top_y]
    type = FunctionNeumannBC
    boundary = top
    variable = disp_y
    function = top_trac_y
  [../]
  [./bottom_x]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
    block = 0
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 2.0
  max_xfem_update = 2
[]

[Outputs]
  exodus = true
  execute_on = TIMESTEP_END
  [xfemcutter]
    type=XFEMCutMeshOutput
    xfem_cutter_uo=cut_mesh

  []
  [./console]
    type = Console
    output_linear = true
  [../]
[]
