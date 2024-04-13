vol_frac = 0.5
E0 = 1
Emin = 1e-4
power = 1

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
    xmin = 0
    xmax = 40
    ymin = 0
    ymax = 40
  []
[]

[Variables]
  [T]
    initial_condition = 100
  []
[]

[AuxVariables]
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

[Kernels]
  [heat]
    type = HeatConduction
    diffusion_coefficient = k
    variable = T
  []
  [heat_source]
    type = HeatSource
    function = 1e-2
    variable = T
  []
[]
[DiracKernels]
  [src]
    type = ConstantPointSource
    variable = T
    point = '0 5 0'
    value = 10
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    variable = T
    boundary = 'right top bottom'
    value = 0.0
  []
[]

[Materials]
  [k]
    type = DerivativeParsedMaterial
    # Emin + (density^penal) * (E0 - Emin)
    expression = '${Emin} + (mat_den ^ ${power}) * (${E0}-${Emin})'
    coupled_variables = 'mat_den'
    property_name = k
  []
  [dc]
    type = ThermalSensitivity
    temperature = T
    design_density = mat_den
    thermal_conductivity = k
  []
  #only needed for objective function output in postprocessor
  [thermal_compliance]
    type = ThermalCompliance
    temperature = T
    thermal_conductivity = k
  []
[]

[UserObjects]
  [rad_avg]
    type = RadialAverage
    radius = 3
    weights = linear
    prop_name = thermal_sensitivity
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

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-8
  dt = 1.0
  dtmin = 1.0
  num_steps = 20
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
  []
  print_linear_residuals = false
[]

[Postprocessors]
  [mesh_volume]
    type = VolumePostprocessor
    execute_on = 'initial timestep_end'
  []
  [total_vol]
    type = ElementIntegralVariablePostprocessor
    variable = mat_den
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [vol_frac]
    type = ParsedPostprocessor
    expression = 'total_vol / mesh_volume'
    pp_names = 'total_vol mesh_volume'
  []
  [sensitivity]
    type = ElementIntegralMaterialProperty
    mat_prop = thermal_sensitivity
  []
  [objective_thermal]
    type = ElementIntegralMaterialProperty
    mat_prop = thermal_compliance
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
