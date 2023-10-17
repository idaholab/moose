power = 2.0

rho0 = 0.0
rho1 = 1.0

TC0 = 1.0e-16
TC1 = 1.0

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

[GlobalParams]
  illumination_flux = '1 1 0'
[]

[AuxVariables]
  [mat_den]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0.1
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
  [thermal_compliance]
    type = ThermalCompliance
    temperature = u
    thermal_conductivity = thermal_cond
    outputs = 'exodus'
  []
  [thermal_cond]
    type = DerivativeParsedMaterial
    expression = "A1:=(${TC0}-${TC1})/(${rho0}^${power}-${rho1}^${power}); "
                 "B1:=${TC0}-A1*${rho0}^${power}; TC1:=A1*mat_den^${power}+B1; TC1"
    coupled_variables = 'mat_den'
    property_name = thermal_cond
    outputs = 'exodus'
  []
  [thermal_compliance_sensitivity]
    type = ThermalSensitivity
    design_density = mat_den
    thermal_conductivity = thermal_cond
    temperature = u
    outputs = 'exodus'
  []
[]

[BCs]
  [flux_u]
    type = DirectionalFluxBC
    variable = u
    boundary = outer
  []

  [flux_v]
    type = DirectionalFluxBC
    variable = v
    boundary = outer
    self_shadow_uo = shadow
  []
[]

[Postprocessors]
  [ave_v_all]
    type = SideAverageValue
    variable = v
    boundary = outer
  []
  [ave_v_exposed]
    type = ExposedSideAverageValue
    variable = v
    boundary = outer
    self_shadow_uo = shadow
  []
[]

[UserObjects]
  [shadow]
    type = SelfShadowSideUserObject
    boundary = outer
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
