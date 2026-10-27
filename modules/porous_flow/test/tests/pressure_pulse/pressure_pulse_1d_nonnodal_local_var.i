# A PorousFlow variable that is element-local (CONSTANT MONOMIAL) is legitimate when it is only
# ever a local unknown, as the adsorbed concentration in the desorption tests is: no nodal Material
# reads it, so the nodal-variable checks in the Materials never see it.  The upwinded and
# mass-lumped kernels, however, identify each element test function with a mesh node, so they cannot
# be applied to such a variable.  This input isolates that: the nodal Materials couple only the
# nodal variable pp, so a kernel applied to conc is the only object that can object.
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
    initial_condition = 2E6
  []
  [conc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
[]

[Kernels]
  # Applied to the nodal variable, so these are always valid
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '0 0 0'
  []
  # A local, non-lumped and non-upwinded equation for the element-local variable, in the spirit of
  # the desorption kernels: valid
  [conc_dot]
    type = TimeDerivative
    variable = conc
  []
  # Mass lumped, applied to the element-local variable: invalid
  [lumped_on_conc]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = conc
  []
  # Fully upwinded, applied to the element-local variable: invalid
  [upwind_on_conc]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = conc
    gravity = '0 0 0'
  []
[]

[BCs]
  # A real pressure pulse, so that the residual is not identically zero: convergence of a null
  # problem depends on whether |R| rounds to exactly zero, which is build dependent.
  [left]
    type = DirichletBC
    boundary = left
    value = 3E6
    variable = pp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp conc'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-7
  []
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
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0
    phase = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E3
  end_time = 1E4
  # Well below the 1E-4 initial residual of the pulse, but far above the roundoff the conc
  # equation sits at, since it has no source
  nl_abs_tol = 1E-10
[]

[Postprocessors]
  # The same points as pressure_pulse_1d, whose physics this input reproduces exactly apart
  # from the extra element-local variable: the flow solution must be unperturbed by it.
  [p000]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [p010]
    type = PointValue
    variable = pp
    point = '10 0 0'
    execute_on = 'initial timestep_end'
  []
  [p020]
    type = PointValue
    variable = pp
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  []
  [p030]
    type = PointValue
    variable = pp
    point = '30 0 0'
    execute_on = 'initial timestep_end'
  []
  [p040]
    type = PointValue
    variable = pp
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  []
  [p050]
    type = PointValue
    variable = pp
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  []
  [p060]
    type = PointValue
    variable = pp
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  []
  [p070]
    type = PointValue
    variable = pp
    point = '70 0 0'
    execute_on = 'initial timestep_end'
  []
  [p080]
    type = PointValue
    variable = pp
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  []
  [p090]
    type = PointValue
    variable = pp
    point = '90 0 0'
    execute_on = 'initial timestep_end'
  []
  [p100]
    type = PointValue
    variable = pp
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  []
  # The local unknown itself, which stays at its initial condition since nothing sources it
  [conc]
    type = ElementAverageValue
    variable = conc
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = pressure_pulse_1d_nonnodal_local_var
  print_linear_residuals = false
  csv = true
[]
