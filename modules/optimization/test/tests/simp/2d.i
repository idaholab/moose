vol_frac = 0.2

[Mesh]
  [planet]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    radii = 1
    num_sectors = 10
    rings = 2
    preserve_volumes = false
  []

  [moon]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    radii = 0.5
    num_sectors = 8
    rings = 2
    preserve_volumes = false
  []
  [combine]
    type = CombinerGenerator
    inputs = 'planet moon'
    positions = '0 0 0 -1.5 -0.5 0'
  []
[]

[AuxVariables]
  [mat_den]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0.1
  []
  [Dc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1.0
  []
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [dt_u]
    type = TimeDerivative
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [dt_v]
    type = TimeDerivative
    variable = v
  []
[]

[Materials]
  [thermal_cond]
    type = GenericFunctionMaterial
    prop_values = '-1.4*abs(y)-2.7*abs(x)'
    prop_names = thermal_cond
    outputs = 'exodus'
  []
  [thermal_compliance_sensitivity]
    type = GenericFunctionMaterial
    prop_values = '-3*abs(y)-1.5*abs(x)'
    prop_names = thermal_sensitivity
    outputs = 'exodus'
  []
[]

[BCs]
  [flux_u]
    type = DirichletBC
    variable = u
    boundary = outer
    value = 3.0
  []

  [flux_v]
    type = DirichletBC
    variable = v
    boundary = outer
    value = 7.0
  []
[]

[UserObjects]
  [rad_avg]
    type = RadialAverage
    radius = 0.1
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

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 15
  nl_rel_tol = 1e-04
[]

[Outputs]
  exodus = true
[]
