[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD8
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Problem]
  coord_type = RZ
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    add_variables = true
    use_automatic_differentiation = true
  []
[]

[AuxVariables]
  [stress_theta]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_theta]
    type = ADRankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = stress_theta
    execute_on = timestep_end
  []
[]

[Materials]
  [elasticity_tensor]
    #Material constants selected to match isotropic lambda and shear modulus case
    type = ADComputeElasticityTensor
    C_ijkl = '1022726 113636 113636 1022726 454545'
    fill_method = axisymmetric_rz
  []
  [elastic_stress]
    type = ADComputeLinearElasticStress
  []
[]

[BCs]
# pin particle along symmetry planes
  [no_disp_r]
    type = DirichletBC
    variable = disp_r
    boundary = left
    value = 0.0
  []

  [no_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []

# exterior and internal pressures
  [exterior_pressure_r]
    type = ADPressure
    variable = disp_r
    boundary = right
    factor = 200000
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '  201               hypre    boomeramg      10'

  line_search = 'none'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 5e-9
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 50

  start_time = 0.0
  end_time = 1
  num_steps = 1000

  dtmax = 5e6
  dtmin = 1

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 6
    iteration_window = 0
    linear_iteration_ratio = 100
  []

  [Predictor]
    type = SimplePredictor
    scale = 1.0
  []

[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[Outputs]
  file_base = 2D-axisymmetric_rz_test_out
  exodus = true
[]
