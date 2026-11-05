# Generic adaptive-substepping regression for a high-power porous-LPS creep law. The first coarse
# local attempt is intentionally too large after convergence, so the a-posteriori controller must
# refine the constitutive step. A second global step then initializes its subdivision from the
# previous accepted admitted substep-control rate.

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
  [constants]
    type = ADGenericConstantMaterial
    prop_names = 'creep_1 creep_3'
    prop_values = '0 1e-24'
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = lps
  []
  [porosity]
    type = ADPorosityFromStrain
    initial_porosity = 0.1
    inelastic_strain = combined_inelastic_strain
  []
  [lps]
    type = ADPorousViscoplasticityStressUpdate
    coefficient = 'creep_1 creep_3'
    power = '1 3'
    initial_porosity = 0.1
    minimum_porosity = 1e-10
    max_inelastic_increment = 1e-2
    use_substepping = INCREMENT_BASED
    substep_strain_tolerance = 1
    adaptive_substepping = true
    maximum_number_substeps = 4
    verbose = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 2
  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  print_linear_residuals = false
  csv = false
[]
