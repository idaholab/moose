# Steady conduction in a heated porous solid slab of length L = 5 with the temperature fixed to
# T_left = 300 on the left and the other boundaries insulated. The effective solid conductivity
# kappa_s includes the solid fraction, so the temperature rise across the slab,
#   T_max - T_left = Q L^2 / (2 kappa_s) = 600 * 25 / 2 = 7500,
# does not depend on the porosity. With the uniform source, the two-point flux of each face equals
# the exact heat flux, so the value in the cell next to the insulated end reproduces this rise.

porosity = 0.25

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = 1
    nx = 20
    ny = 10
  []
[]

[Physics]
  [NavierStokes]
    [SolidHeatTransfer]
      [solid]
        block = 0

        thermal_conductivity_solid = 'k_solid'
        fixed_temperature_boundaries = 'left'
        boundary_temperatures = '300'

        # Heat source per unit total volume of 600
        external_heat_source = '300'
        external_heat_source_coeff = 2
      []
    []
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = ${porosity}
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'k_solid cp_solid rho_solid'
    prop_values = '1 1000 1000'
  []
[]

[Postprocessors]
  [T_max]
    type = ElementExtremeValue
    variable = T_solid
  []
  [temperature_rise]
    type = ParsedPostprocessor
    expression = 'T_max - 300'
    pp_names = 'T_max'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
