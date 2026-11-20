# Uniform heating of an insulated porous solid with a temperature-dependent specific heat
# cp_s = a + b T, solved with the linear finite volume solid heat transfer Physics. The temperature
# stays uniform, so the energy balance (1 - eps) rho_s dh_s/dt = Q with h_s = a T + b T^2 / 2 gives
#   a T + b T^2 / 2 = a T_0 + b T_0^2 / 2 + Q t / ((1 - eps) rho_s).
# At t = 10 the enthalpy has risen by 5e6 * 10 / 500 = 1e5 and T = 384.429. Fixed-point iterations
# within each time step evaluate cp_s at the new temperature, so the BDF2 error decreases with the
# square of the time step.

a = 500
b = 2
T_0 = 300
porosity = 0.5
rho_s = 1000
Q = 5e6

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
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
        thermal_conductivity_solid = 1
        rho_solid = ${rho_s}
        cp_solid = 'cp_s'
        initial_temperature = ${T_0}
        external_heat_source = ${Q}
      []
    []
  []
[]

[FunctorMaterials]
  [cp]
    type = ParsedFunctorMaterial
    property_name = 'cp_s'
    functor_names = 'T_solid'
    functor_symbols = 'T'
    expression = '${a} + ${b} * T'
  []
[]

[Postprocessors]
  [T_avg]
    type = ElementAverageValue
    variable = T_solid
  []
  [T_exact]
    type = ParsedPostprocessor
    expression = 'h := ${a} * ${T_0} + ${b} * ${T_0}^2 / 2 + ${Q} * t / ((1 - ${porosity}) * ${rho_s});
                  (-${a} + sqrt(${a}^2 + 2 * ${b} * h)) / ${b}'
    use_t = true
  []
  [relative_error]
    type = ParsedPostprocessor
    expression = 'abs(T_avg - T_exact) / T_exact'
    pp_names = 'T_avg T_exact'
  []
[]

[Convergence]
  # The fixed-point iterations on cp_s reach round-off in 6 iterations at this time step
  [cp_iterations]
    type = IterationCountConvergence
    max_iterations = 6
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Transient
  system_names = solid_energy_system
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = cp_iterations
  scheme = bdf2
  num_steps = 10
  dt = 1
  l_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
