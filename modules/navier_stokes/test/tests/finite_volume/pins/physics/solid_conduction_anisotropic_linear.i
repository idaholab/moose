# Steady conduction in a heated porous solid slab of height H = 5 with an anisotropic effective
# solid conductivity kappa_s = (1, 4), solved with the linear finite volume solid heat transfer
# Physics. The temperature is fixed to T_bottom = 300 at the bottom and the other boundaries are
# insulated, so heat flows only along y and the temperature rise across the slab is
#   T_max - T_bottom = Q H^2 / (2 kappa_s,y) = 600 * 25 / 8 = 1875.
# A conductivity taken from the x component would give a rise four times larger. With the uniform
# source, the two-point flux of each face equals the exact heat flux, so the value in the cell next
# to the insulated end reproduces this rise.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 5
    nx = 10
    ny = 20
  []
[]

[Problem]
  linear_sys_names = 'solid_energy_system'
[]

[Physics]
  [NavierStokes]
    [SolidHeatTransferSegregated]
      [solid]
        porosity = 0.25
        thermal_conductivity_solid = 'k_solid'
        rho_solid = 1000
        cp_solid = 1000
        fixed_temperature_boundaries = 'bottom'
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
  [conductivity]
    type = GenericVectorFunctorMaterial
    prop_names = 'k_solid'
    prop_values = '1 4 1'
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
