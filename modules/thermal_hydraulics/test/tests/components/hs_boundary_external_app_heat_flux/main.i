# Main input file.
#
# Run mesh.i first to produce a mesh file that this input uses:
#
#   thermal_hydraulics-opt -i mesh.i --mesh-only mesh.e

length = 5.0
n_elems_axial = 10

rho_name = density
cp_name = specific_heat
k_name = thermal_conductivity
rho = 8000.0
cp = 500.0
k = 15.0

T_initial = 500.0
power = 1000.0

[Mesh]
  type = FileMesh
  file = mesh.e
[]

[Variables]
  [T_solid]
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T_solid
    value = ${T_initial}
  []
[]

[Kernels]
  [time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = T_solid
    density_name = ${rho_name}
    specific_heat = ${cp_name}
  []
  [heat_conduction]
    type = ADHeatConduction
    variable = T_solid
    thermal_conductivity = ${k_name}
  []
[]

[BCs]
  [bc]
    type = FunctorNeumannBC
    variable = T_solid
    boundary = 'inner'
    functor = heat_flux_fn
    flux_is_inward = false
  []
[]

[Materials]
  [ad_constant_mat]
    type = ADGenericConstantMaterial
    prop_names = '${rho_name} ${cp_name} ${k_name}'
    prop_values = '${rho} ${cp} ${k}'
  []
[]

[Functions]
  [heat_flux_fn]
    type = ParsedFunction
    symbol_names = 'S'
    symbol_values = 'inner_surface_area'
    expression = '${power} / S'
  []
[]

[Postprocessors]
  [inner_surface_area]
    type = AreaPostprocessor
    boundary = 'inner'
    execute_on = 'INITIAL'
  []
  [inner_perimeter]
    type = ParsedPostprocessor
    pp_names = 'inner_surface_area'
    function = 'inner_surface_area / ${length}'
    execute_on = 'INITIAL'
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = 'sub.i'
    positions = '0 0 0'
    max_procs_per_app = 1
    output_in_position = true
    execute_on = 'TIMESTEP_END'
  []
[]

[UserObjects]
  [layered_average_heat_flux]
    type = NearestPointLayeredSideAverageFunctor
    direction = z
    points='0 0 0'
    num_layers = ${n_elems_axial}
    functor = heat_flux_fn
    boundary = 'inner'
    execute_on = 'TIMESTEP_END'
  []
[]

[Transfers]
  [heat_flux_transfer]
    type = MultiAppGeneralFieldUserObjectTransfer
    to_multi_app = sub
    source_user_object = layered_average_heat_flux
    variable = q_ext
  []
  [perimeter_transfer]
    type = MultiAppPostprocessorTransfer
    to_multi_app = sub
    from_postprocessor = inner_perimeter
    to_postprocessor = P_ext
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  scheme = bdf2
  dt = 10.0
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = NEWTON
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10
[]
