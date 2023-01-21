# This is a test of the usage of initial adaptivity with contact.
# It ensures that contact is enforced on the new nodes that are
# created due to refinement on the secondary side of the interface.

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = 2blocks.e
  patch_size = 80
  parallel_type = replicated
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
    expression = -t/10
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
  []
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
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = negramp
  [../]
  [./right_y]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stiffStuff1_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Contact]
  [./leftright]
    secondary = 3
    primary = 2
    model = frictionless
    penalty = 1e+6
    normal_smoothing_distance = 0.1
  [../]
[]

[Adaptivity]
  steps = 0
  marker = box
  max_h_level = 2
  initial_steps = 2
  [./Markers]
    [./box]
      type = BoxMarker
      bottom_left = '0.5 -2.0 0.0'
      top_right = '0.75 2.0 0.0'
      inside = refine
      outside = do_nothing
    [../]
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       superlu_dist'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 1000
  dt = 0.2
  end_time = 1.0
  l_tol = 1e-6
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-9
[]

[Outputs]
  exodus = true
  console = true
[]
