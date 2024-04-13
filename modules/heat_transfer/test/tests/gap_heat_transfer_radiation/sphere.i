#
# This problem is one of radiation boundary conditions between two
# spherical surfaces.
#
#            S(T1^4 - T2^4)                         R1^2
# flux1 = - ----------------   and flux2 = -flux1 * ----
#           1    1 - e2   R1^2                      R2^2
#           -- + ------ * ----
#           e1     e2     R2^2
#
# where S is the Stefan Boltzmann constant         5.67e-8 W/m^2/K^4
#       T1 is the temperature on the left surface  278 K
#       T2 is the temperature on the right surface 333 K
#       e1 is the emissivity for the left surface  0.8
#       e2 is the emissivity for the left surface  0.9
#       R1 is the radius of the inner surface      0.1 m
#       R2 is the radius of the outer surface      0.11 m
#
# Flux1:
# Exact           Code
# -------------   -------------
# -267.21 W/m^2   -267.02 W/m^2
#
# Flux2:
# Exact           Code
# -------------   -------------
#  220.83 W/m^2    220.70 W/m^2
#

thick = 0.01
R1 = 0.1
R2 = 0.11

[GlobalParams]
  order = second
  family = lagrange
[]

[Mesh]
  coord_type = RSPHERICAL
  [mesh1]
    type = GeneratedMeshGenerator
    dim = 1
    elem_type = edge3
    nx = 4
    xmin = '${fparse R1 - thick}'
    xmax = '${R1}'
    boundary_name_prefix = left
  []
  [mesh2]
    type = GeneratedMeshGenerator
    dim = 1
    elem_type = edge3
    nx = 4
    ny = 1
    xmin = '${R2}'
    xmax = '${fparse R2 + thick}'
    boundary_id_offset = 4
    boundary_name_prefix = right
  []
  [final]
    type = CombinerGenerator
    inputs = 'mesh1 mesh2'
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left_left
    value = 278
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = right_right
    value = 333
  []
[]

[Materials]
  [heat]
    type = HeatConductionMaterial
    thermal_conductivity = 200 # W/m/K
    specific_heat = 4.2e5
  []
[]

[ThermalContact]
  [thermal_contact]
    type = GapHeatTransfer
    variable = temperature
    primary = left_right
    secondary = right_left
    emissivity_primary = 0.8
    emissivity_secondary = 0.9
    quadrature = true
    gap_conductivity = 1e-40 # requires a positive value
    gap_geometry_type = sphere
  []
[]

[Functions]
  [analytic_flux_1]
    type = ParsedFunction
    symbol_names = 'S        T1  T2  e1  e2  R1    R2'
    symbol_values = '5.67e-8 278 333 0.8 0.9 ${R1} ${R2}'
    expression = 'T14 := T1*T1*T1*T1;
                  T24 := T2*T2*T2*T2;
                  S*(T14-T24)/(1/e1+(1-e2)/e2*R1*R1/R2/R2)'
  []
  [analytic_flux_2]
    type = ParsedFunction
    symbol_names = 'S        T1  T2  e1  e2  R1    R2'
    symbol_values = '5.67e-8 278 333 0.8 0.9 ${R1} ${R2}'
    expression = 'T14 := T1*T1*T1*T1;
                  T24 := T2*T2*T2*T2;
                  -S*(T14-T24)/(1/e1+(1-e2)/e2*R1*R1/R2/R2)*R1*R1/R2/R2'
  []
[]

[Postprocessors]
  [code_flux_1]
    type = SideDiffusiveFluxAverage
    variable = temperature
    boundary = left_right
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  []
  [analytic_flux_1]
    type = FunctionValuePostprocessor
    function = analytic_flux_1
    execute_on = 'initial timestep_end'
  []
  [error_1]
    type = ParsedPostprocessor
    pp_names = 'code_flux_1 analytic_flux_1'
    expression = '(analytic_flux_1 - code_flux_1)/analytic_flux_1*100'
    execute_on = 'initial timestep_end'
  []
  [code_flux_2]
    type = SideDiffusiveFluxAverage
    variable = temperature
    boundary = right_left
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  []
  [analytic_flux_2]
    type = FunctionValuePostprocessor
    function = analytic_flux_2
    execute_on = 'initial timestep_end'
  []
  [error_2]
    type = ParsedPostprocessor
    pp_names = 'code_flux_2 analytic_flux_2'
    expression = '(analytic_flux_2 - code_flux_2)/analytic_flux_2*100'
    execute_on = 'initial timestep_end'
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
  solve_type = newton

  num_steps = 1
  dt = 1
  end_time = 1

  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-10
[]

[Outputs]
  csv = true
[]
