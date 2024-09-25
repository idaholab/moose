vol_frac = 0.5
E0 = 1
Emin = 1e-8
power = 2
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 150
    ny = 50
    xmin = 0
    xmax = 30
    ymin = 0
    ymax = 10
  []
  [node]
    type = ExtraNodesetGenerator
    input = MeshGenerator
    new_boundary = pull
    nodes = 0
  []
  [push]
    type = ExtraNodesetGenerator
    input = node
    new_boundary = push
    coord = '30 10 0'
  []

[]

[AuxVariables]

  [Emin]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${Emin}
  []
  [power]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${power}
  []
  [E0]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${E0}
  []
  [Dc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1.0
  []
  [mat_den]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${vol_frac}
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    add_variables = true
    incremental = false
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    variable = disp_y
    boundary = pull
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  []

[]
[NodalKernels]
  [pull]
    type = NodalGravity
    variable = disp_y
    boundary = push
    gravity_value = -1
    mass = 1
  []
[]
[Materials]
  [elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    youngs_modulus = E_phys
    poissons_ratio = poissons_ratio
    args = 'Emin mat_den power E0'
  []
  [E_phys]
    type = DerivativeParsedMaterial
    # Emin + (density^penal) * (E0 - Emin)
    expression = '${Emin} + (mat_den ^ ${power}) * (${E0}-${Emin})'
    coupled_variables = 'mat_den'
    property_name = E_phys
  []
  [poissons_ratio]
    type = GenericConstantMaterial
    prop_names = poissons_ratio
    prop_values = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [dc]
    type = ComplianceSensitivity
    design_density = mat_den
    youngs_modulus = E_phys
    incremental = false
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
[UserObjects]
  [rad_avg]
    type = RadialAverage
    radius = 1.2
    weights = linear
    prop_name = sensitivity
    execute_on = TIMESTEP_END
    force_preaux = true
  []
  [update]
    type = DensityUpdate
    density_sensitivity = Dc
    design_density = mat_den
    volume_fraction = ${vol_frac}
    execute_on = TIMESTEP_BEGIN
  []
  [calc_sense]
    type = SensitivityFilter
    density_sensitivity = Dc
    design_density = mat_den
    filter_UO = rad_avg
    execute_on = TIMESTEP_END
    force_postaux = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-8
  dt = 1.0
  num_steps = 70
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'TIMESTEP_END'
  []
  print_linear_residuals = false
[]

[Postprocessors]
  [total_vol]
    type = ElementIntegralVariablePostprocessor
    variable = mat_den
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
