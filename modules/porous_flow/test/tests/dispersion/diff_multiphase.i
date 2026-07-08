# Regression test for the per-phase, per-component indexing of diffusion
# coefficients in PorousFlowDiffusivityBase.
#
# Two fluid phases and two fluid components.  Saturation, pressure and the
# phase-1 composition (massfrac1) are held fixed as auxiliary fields, leaving
# the phase-0 composition (massfrac0) as the only nonlinear variable, so the
# component-1 mass balance is a single, robustly-converging diffusion equation.
# A single implicit timestep is taken from a non-equilibrium initial state, with
# disp_long = disp_trans = 0 and gravity = 0 so that only molecular diffusion
# transports mass.
#
# The diffusive flux of component 1 sums a phase-0 term (driven by the massfrac0
# gradient) and a phase-1 term (driven by the fixed massfrac1 gradient).
# massfrac1 is given a curved profile so that its gradient varies in space: the
# phase-1 diffusive flux then has a non-zero divergence and acts as a distributed
# source in the interior, in direct proportion to the phase-1 diffusion
# coefficient D[1][1].
#
# The four diffusion coefficients are deliberately distinct.  They are read with
# the index ph * num_components + comp.  An incorrect index (e.g. ph + comp)
# selects the wrong coefficient for phase 1 and changes the massfrac0 profile.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [massfrac0]
  []
[]

[AuxVariables]
  [sgas]
    initial_condition = 0.5
  []
  [massfrac1]
    # First-order Lagrange so the imposed curved profile has a spatially
    # varying gradient
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = FunctionIC
      function = '0.1 + 0.8 * x * x'
    []
  []
[]

[ICs]
  [massfrac0]
    type = ConstantIC
    variable = massfrac0
    value = 0.5
  []
[]

[BCs]
  [massfrac0_left]
    type = DirichletBC
    variable = massfrac0
    boundary = left
    value = 1
  []
  [massfrac0_right]
    type = DirichletBC
    variable = massfrac0
    boundary = right
    value = 0
  []
[]

[Kernels]
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = massfrac0
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = massfrac0
    disp_long = '0 0'
    disp_trans = '0 0'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'massfrac0'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1e7
    density0 = 10
    thermal_expansion = 0
    viscosity = 1
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 1e7
    density0 = 10
    thermal_expansion = 0
    viscosity = 0.1
  []
[]

[Materials]
  [temp]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = 1e5
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac0 massfrac1'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [poro]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [diff]
    # Distinct coefficients.  Order is component 0 in phase 0, component 1 in
    # phase 0, component 0 in phase 1, component 1 in phase 1:
    #   phase 0 -> '1e-2 2e-2', phase 1 -> '5e-2 1e-1'
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-2 2e-2 5e-2 1e-1'
    tortuosity = '0.1 0.2'
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-9 0 0 0 1e-9 0 0 0 1e-9'
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityConst
    phase = 1
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
  solve_type = NEWTON
  dt = 1
  num_steps = 1
[]

[VectorPostprocessors]
  [xmass]
    type = NodalValueSampler
    sort_by = id
    variable = massfrac0
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = final
  []
[]
