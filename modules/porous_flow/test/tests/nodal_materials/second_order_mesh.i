# A FIRST-order porepressure on a SECOND-order (TET10) mesh.
#
# The nodal Materials must visit only the 4 nodes that carry a porepressure
# degree of freedom, not all 10 nodes of the element.  Before the fix they
# looped over _current_elem->n_nodes() and read past the end of the 4-entry
# array of nodal porepressure values.  That read is silent in an optimised
# build, so this test is only meaningful under a devel or dbg build, where
# MooseArray's bounds assertion catches it.
#
# There is no mechanics and no mixed order here: a single first-order variable
# on a second-order mesh is enough to trigger it.

[Mesh]
  [mymesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    elem_type = TET10
  []
  second_order = true
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [porepressure]
    family = LAGRANGE
    order = FIRST # FIRST on a SECOND-order mesh: 4 DOFs against 10 nodes
    initial_condition = 1e6
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = porepressure
  []
  [flux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = porepressure
    gravity = '0 0 0'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = porepressure
    boundary = left
    value = 2e6
  []
  [right]
    type = DirichletBC
    variable = porepressure
    boundary = right
    value = 1e6
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure'
    number_fluid_phases = 1
    number_fluid_components = 1
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
    type = PorousFlow1PhaseFullySaturated
    porepressure = porepressure
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
    permeability = '1e-14 0 0  0 1e-14 0  0 0 1e-14'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
    kr = 1.0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    solve_type = NEWTON
  []
[]

[Postprocessors]
  # The fluid mass is an integral of the nodal fluid density, porosity and
  # saturation, so it changes if the nodal Material values are wrong
  [fluid_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [pp_centre]
    type = PointValue
    variable = porepressure
    point = '0.5 0.5 0.5'
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
[]
