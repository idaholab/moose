# Steady conduction in a heated porous solid slab of length L = 5, solved with the linear finite
# volume solid heat transfer Physics. The temperature is fixed to T_left = 300 on the left and the
# other boundaries are insulated. With the effective solid conductivity kappa_s, the temperature
# rise across the slab,
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

[Problem]
  linear_sys_names = 'solid_energy_system'
[]

[Physics]
  [NavierStokes]
    [SolidHeatTransferSegregated]
      [solid]
        porosity = ${porosity}
        thermal_conductivity_solid = 'k_solid'
        rho_solid = 1000
        cp_solid = 1000
        fixed_temperature_boundaries = 'left'
        boundary_temperatures = '300'

        # Heat source per unit total volume of 600
        external_heat_source = '300'
        external_heat_source_coeff = 2

        use_nonorthogonal_correction = false
      []
    []
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'k_solid'
    prop_values = '1'
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
  system_names = solid_energy_system
  l_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
