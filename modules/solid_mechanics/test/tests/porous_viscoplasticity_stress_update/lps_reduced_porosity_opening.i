# Generic scalar reduced-porosity globalization regression. A test-only linear pore-pressure
# closure starts highly overpressurized at the physical porosity floor and decreases as porosity
# opens. With the coupled Newton deliberately limited to one iteration, the generic reduced-f
# solver must recover and certify the FREE root without relying on a BISON gas EOS.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmax = 1e-3
  ymax = 1e-3
[]

[Physics/SolidMechanics/QuasiStatic/All]
  strain = FINITE
  add_variables = true
  use_automatic_differentiation = true
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = disp_x
    boundary = right
    value = 0
  []
  [bottom]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [top]
    type = ADDirichletBC
    variable = disp_y
    boundary = top
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  []
  [creep_coefficient]
    type = ADGenericConstantMaterial
    prop_names = creep_coefficient
    prop_values = 2.5e-9
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = lps
  []
  [porosity]
    type = ADPorosityFromStrain
    initial_porosity = 1e-10
    inelastic_strain = combined_inelastic_strain
  []
  [lps]
    type = ADPorousViscoplasticityStressUpdateTest
    coefficient = creep_coefficient
    power = 1
    initial_porosity = 1e-10
    minimum_porosity = 1e-10
    max_inelastic_increment = 1
    use_prescribed_scalar_pressure = true
    scalar_pressure_reference_porosity = 1e-10
    scalar_pressure = 2e8
    scalar_pressure_derivative = -2e17
    local_newton_tolerance = 1e-8
    local_newton_stagnation_tolerance = 1e-7
    local_newton_max_iterations = 1
    porosity_bound_tolerance = 1e-12
    reduced_porosity_probe_growth = 4
    reduced_porosity_max_probes = 24
    reduced_porosity_root_max_iterations = 64
    use_substepping = NONE
    verbose = true
  []
[]

[Postprocessors]
  [porosity]
    type = ADElementAverageMaterialProperty
    mat_prop = porosity
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[UserObjects]
  [check_opening]
    type = Terminator
    expression = 'porosity != porosity | porosity <= 1e-10 | porosity >= 1'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'Generic reduced-porosity globalization did not produce a finite opening state.'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  num_steps = 1
  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = false
  print_linear_residuals = false
[]
