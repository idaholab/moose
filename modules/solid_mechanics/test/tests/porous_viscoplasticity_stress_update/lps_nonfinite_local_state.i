# Generic nonfinite-state rejection regression. An intentionally enormous n=4.5 coefficient
# overflows the trial creep response under simple shear. The constitutive update must throw a
# recoverable MooseException before committing nonfinite strain or stress.

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

[Functions]
  [shear]
    type = ParsedFunction
    expression = '3e-5 * t'
  []
[]

[BCs]
  [bottom_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [bottom_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [top_x]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = top
    function = shear
  []
  [top_y]
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
    prop_values = 1e300
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = lps
  []
  [porosity]
    type = ADPorosityFromStrain
    initial_porosity = 0.05
    inelastic_strain = combined_inelastic_strain
  []
  [lps]
    type = ADPorousViscoplasticityStressUpdate
    coefficient = creep_coefficient
    power = 4.5
    initial_porosity = 0.05
    minimum_porosity = 1e-10
    max_inelastic_increment = 1e-2
    use_substepping = NONE
    local_newton_tolerance = 1e-12
    local_newton_stagnation_tolerance = 1e-12
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  dtmin = 1
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
