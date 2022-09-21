## This is an example input file showing how to set a Type I (Dirichlet) BC with PorousFlowPiecewiseLinearSink
##
## Problem setup:
##   - The boundaries are set to P(x = 0) = 2e6 Pa, P(x = 1) = 1e6 and run to steady state.
##   - The 2d domain is 1 m x 1 m
##   - The permeability is set to 1E-15 m2, fluid viscosity = 1E-3 Pa-s
##   - The steady state flux is calculated q = -k/mu*grad(P) = 1e-6 m/s
##
## Problem verification (in csv output):
##   - The flux in and out of the domain are 1e-6 m/s (matching steady state solution)
##   - The pressure at the left and right boundaries are set to 2e6 and 1e6 Pa, respectively

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  xmin = 0
  xmax = 1
  ny = 2
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [porepressure]
    initial_condition = 1.5e6 # initial pressure in domain
  []
[]

[PorousFlowBasicTHM]
  porepressure = porepressure
  coupling_type = Hydro
  gravity = '0 0 0'
  fp = the_simple_fluid
[]

[AuxVariables]
  [fluxes_out]
  []
  [fluxes_in]
  []
[]

[BCs]
  [in_left]
    type = PorousFlowPiecewiseLinearSink
    variable = porepressure
    boundary = 'left'
    pt_vals = '-1e9 1e9' # x coordinates defining g
    multipliers = '-1e9 1e9' # y coordinates defining g
    PT_shift = 2.E6   # BC pressure
    flux_function = 1E-5 # Variable C
    fluid_phase = 0
    save_in = fluxes_out
  []
  [out_right]
    type = PorousFlowPiecewiseLinearSink
    variable = porepressure
    boundary = 'right'
    pt_vals = '-1e9 1e9' # x coordinates defining g
    multipliers = '-1e9 1e9' # y coordinates defining g
    PT_shift = 1.E6   # BC pressure
    flux_function = 1E-6 # Variable C
    fluid_phase = 0
    save_in = fluxes_in
  []
[]

[Postprocessors]
  [left_flux]
    type = NodalSum
    boundary = 'left'
    variable = fluxes_out
    execute_on = 'timestep_end'
  []
  [right_flux]
    type = NodalSum
    boundary = 'right'
    variable = fluxes_in
    execute_on = 'timestep_end'
  []
  [left_pressure]
    type = SideAverageValue
    boundary = 'left'
    variable = porepressure
    execute_on = 'timestep_end'
  []
  [right_pressure]
    type = SideAverageValue
    boundary = 'right'
    variable = porepressure
    execute_on = 'timestep_end'
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0
    viscosity = 1.0E-3
    density0 = 1000.0
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    biot_coefficient = 0.8
    solid_bulk_compliance = 2E-7
    fluid_bulk_modulus = 1E7
  []
  [permeability_aquifer]
    type = PorousFlowPermeabilityConst
    permeability = '1E-15 0 0   0 1E-15 0   0 0 1E-15'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E6
  dt = 1E5
  nl_abs_tol = 1E-10
[]

[Outputs]
  csv = true
[]
