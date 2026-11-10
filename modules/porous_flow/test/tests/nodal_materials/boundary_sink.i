# A nodal Material evaluated on a BOUNDARY: a sink that reads the nodal fluid
# density, viscosity and relative permeability on the "right" sideset.
#
# A boundary Material's _current_elem is still the volume element, and the
# PorousFlowSink family indexes the nodal properties by the test-function
# index _i, which runs over the degrees of freedom of the element and not over
# the nodes of the face.  The node count used by the nodal Materials must
# therefore be the same element degree-of-freedom count that is used in the
# interior.
#
# This input is run twice: as written on a TET4 mesh, and again with the mesh
# overridden to TET10.  The porepressure remains FIRST order in both, so the
# two solve the identical discrete problem and share a gold file.

[Mesh]
  [mymesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = TET4
  []
  second_order = false
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [porepressure]
    family = LAGRANGE
    order = FIRST
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
  [rightsink]
    # use_mobility and use_relperm make this read the nodal density, viscosity
    # and relative permeability on the boundary
    type = PorousFlowPiecewiseLinearSink
    variable = porepressure
    boundary = right
    pt_vals = '0 1e9'
    multipliers = '0 1e9'
    fluid_phase = 0
    flux_function = 1e-6
    use_mobility = true
    use_relperm = true
    PT_shift = 1e6
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
  dt = 100
  end_time = 300
  nl_abs_tol = 1e-12
[]

[Outputs]
  csv = true
[]
