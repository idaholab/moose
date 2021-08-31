[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
[]

[Mesh]
  file = indenter_rz_fine_bigsideset.e
  displacements = 'disp_x disp_y'
  second_order = true
[]

[Functions]
  [disp_y]
    type = PiecewiseLinear
    x = '0.  1.0     1.8    2.   3.0'
    y = '0.  -4.5   -5.4   -5.4  -4.0'
  []
  [force_y]
    type = ParsedFunction
    value = 'if(t < 0.8, 0.0, (-t+ 0.7)*550000.0)'
  []
[]

[Variables]
  [disp_x]
    order = SECOND
    family = LAGRANGE
  []

  [disp_y]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxVariables]
  [saved_x]
    order = SECOND
    family = LAGRANGE
  []
  [saved_y]
    order = SECOND
    family = LAGRANGE
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    block = '1 2'
    use_automatic_differentiation = false
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_zz'
    save_in = 'saved_x saved_y'
  []
[]

[Controls]
  [period0]
    type = TimePeriod
    disable_objects = 'BCs::disp_y'
    start_time = '0.85'
    end_time = '7.0'
    execute_on = 'initial timestep_begin linear nonlinear timestep_end'
  []
[]

[NodalKernels]
  [force_x]
    type = UserForcingFunctionNodalKernel
    variable = disp_y
    boundary = 1
    function = force_y
  []
[]

[BCs]
  # Symmetries of the Problem
  [symm_x_indenter]
    type = DirichletBC
    variable = disp_x
    boundary = 5
    value = 0.0
  []

  [symm_x_material]
    type = DirichletBC
    variable = disp_x
    boundary = 9
    value = 0.0
  []

  [material_base_y]
    type = DirichletBC
    variable = disp_y
    boundary = 8
    value = 0.0
  []

  # Drive indenter motion
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1
    function = disp_y
  []
[]

[Contact]
  [dummy_name]
    primary = 6
    secondary = 4
    model = coulomb
    formulation = kinematic
    normalize_penalty = true
    friction_coefficient = 0.8
    penalty = 8e6 # Increase it was 8e6 working
    tangential_tolerance = 0.005
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.0e7
    poissons_ratio = 0.25
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []

  [tensor_2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  []

  [power_law_hardening]
    type = IsotropicPowerLawHardeningStressUpdate
    strength_coefficient = 1e5
    strain_hardening_exponent = 0.5
    block = '2'
  []
  [radial_return_stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'power_law_hardening'
    tangent_operator = elastic
    block = '2'
  []
  # Materials
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  automatic_scaling = true
  nl_abs_tol = 1.5e-07
  nl_rel_tol = 1.5e-07
  l_max_its = 40
  start_time = 0.0
  dt = 0.01
  end_time = 4.0 # 6.95 first order mesh
[]

[Postprocessors]
  [maxdisp]
    type = NodalVariableValue
    nodeid = 39 # 40-1 where 40 is the exodus node number of the top-left node
    variable = disp_y
  []
  [resid_y]
    type = NodalSum
    variable = saved_y
    boundary = 1
  []
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
  [chkfile]
    type = CSV
    file_base = indenter_rz_nodeface_friction_kinematic_second_order_chkfile
    show = 'maxdisp resid_y'
    execute_on = 'FINAL'
  []
  perf_graph = true
  csv = true # Outputs
[]
