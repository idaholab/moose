# Fast generic integration-equivalence regression for porous LPS substepping.
#
# The candidate advances one global interval and adaptively refines it into multiple accepted local
# constitutive substeps. The reference advances the same linear loading history through four
# quarter-size global steps with local substepping disabled. The accompanying comparison checks the
# final constitutive/mechanical state rather than reproducing a long 500-step transient.

dt = 1
substepping = INCREMENT_BASED
adaptive = true
verbose = false

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
  generate_output = 'hydrostatic_stress vonmises_stress'
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
    prop_values = 1e-24
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
    coefficient = creep_coefficient
    power = 3
    initial_porosity = 0.1
    minimum_porosity = 1e-10
    max_inelastic_increment = 1e-2
    use_substepping = ${substepping}
    substep_strain_tolerance = 1
    adaptive_substepping = ${adaptive}
    verbose = ${verbose}
  []
[]

[Postprocessors]
  [avg_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
    execute_on = TIMESTEP_END
  []
  [avg_vonmises]
    type = ElementAverageValue
    variable = vonmises_stress
    execute_on = TIMESTEP_END
  []
  [gauge_stress]
    type = ADElementAverageMaterialProperty
    mat_prop = gauge_stress
    execute_on = TIMESTEP_END
  []
  [eff_creep_strain]
    type = ADElementAverageMaterialProperty
    mat_prop = effective_viscoplasticity
    execute_on = TIMESTEP_END
  []
  [porosity]
    type = ADElementAverageMaterialProperty
    mat_prop = porosity
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = ${dt}
  end_time = 1
  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  print_linear_residuals = false
  csv = true
  execute_on = FINAL
[]
