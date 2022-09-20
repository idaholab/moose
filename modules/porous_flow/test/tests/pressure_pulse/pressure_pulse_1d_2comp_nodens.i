# No-density version of pressure pulse in 1D with 1 phase but 2 components (viscosity, relperm, etc are independent of mass-fractions)
# This input file uses the PorousFlowFullySaturated Action but with multiply_by_density = false
# This implies the porepressure will immediately go to steady state
# The massfrac variables will then advect with the Darcy velocity
# The Darcy velocity = (k / mu) * grad(P) = (1E-7 / 1E-3) * (1E6 / 1E2) = 1 m/s
# The advection speed = Darcy velocity / porosity = 10 m/s
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    initial_condition = 0
  []
  [tracer]
    initial_condition = 0.1
  []
[]

[PorousFlowFullySaturated]
  porepressure = pp
  mass_fraction_vars = 'tracer'
  gravity = '0 0 0'
  fp = simple_fluid
  stabilization = Full
  multiply_by_density = false
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-7 0 0 0 1E-7 0 0 0 1E-7'
  []
[]

[BCs]
  [left_p]
    type = DirichletBC
    boundary = left
    value = 1E6
    variable = pp
  []
  [right_p]
    type = DirichletBC
    boundary = right
    value = 0
    variable = pp
  []
  [left_tracer]
    type = DirichletBC
    boundary = left
    value = 0.9
    variable = tracer
  []
  [right_tracer]
    type = DirichletBC
    boundary = right
    value = 0.1
    variable = tracer
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 5
[]

[Postprocessors]
  [p000]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [p050]
    type = PointValue
    variable = pp
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  []
  [p100]
    type = PointValue
    variable = pp
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_000]
    type = PointValue
    variable = tracer
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_010]
    type = PointValue
    variable = tracer
    point = '10 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_020]
    type = PointValue
    variable = tracer
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_030]
    type = PointValue
    variable = tracer
    point = '30 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_040]
    type = PointValue
    variable = tracer
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_050]
    type = PointValue
    variable = tracer
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_060]
    type = PointValue
    variable = tracer
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_070]
    type = PointValue
    variable = tracer
    point = '70 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_080]
    type = PointValue
    variable = tracer
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_090]
    type = PointValue
    variable = tracer
    point = '90 0 0'
    execute_on = 'initial timestep_end'
  []
  [tracer_100]
    type = PointValue
    variable = tracer
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
