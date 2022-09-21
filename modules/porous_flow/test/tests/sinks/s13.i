# Apply a PorousFlowOutflowBC to the right-hand side and watch fluid flow to it
#
# This test has a single phase with two components.  The test initialises with
# the porous material fully filled with component=1.  The left-hand side is fixed
# at porepressure=1 and mass-fraction of the zeroth component being unity.
# The right-hand side has
# - porepressure fixed at zero via a DirichletBC: physically this removes component=1
#   to ensure that porepressure remains fixed
# - a PorousFlowOutflowBC for the component=0 to allow that component to exit the boundary freely
#
# Therefore, the zeroth fluid component will flow from left to right (down the
# pressure gradient).
#
# The important DE is
# porosity * dc/dt = (perm / visc) * grad(P) * grad(c)
# which is true for c = mass-fraction, and very large bulk modulus of the fluid.
# For grad(P) constant in time and space (as in this example) this is just the
# advection equation for c, with velocity = perm / visc / porosity.  The parameters
# are chosen to velocity = 1 m/s.
# In the numerical world, and especially with full upwinding, the advection equation
# suffers from diffusion.  In this example, the diffusion is obvious when plotting
# the mass-fraction along the line, but the average velocity of the front is still
# correct at 1 m/s.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
  []
  [frac]
  []
[]

[PorousFlowFullySaturated]
  fp = simple_fluid
  porepressure = pp
  mass_fraction_vars = frac
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = 1-x
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1e10 # need large in order for constant-velocity advection
    density0 = 1 # irrelevant
    thermal_expansion = 0
    viscosity = 11
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.1 0 0 0 1.1 0 0 0 1.1'
  []
[]


[BCs]
  [lhs_fixed_b]
    type = DirichletBC
    boundary = left
    variable = pp
    value = 1
  []
  [rhs_fixed_b]
    type = DirichletBC
    boundary = right
    variable = pp
    value = 0
  []
  [lhs_fixed_a]
    type = DirichletBC
    boundary = left
    variable = frac
    value = 1
  []
  [outflow_a]
    type = PorousFlowOutflowBC
    boundary = right
    include_relperm = false # no need for relperm in this fully-saturated simulation
    mass_fraction_component = 0
    variable = frac
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'asm lu NONZERO 2'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E-2
  end_time = 1
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12
[]

[VectorPostprocessors]
  [mf]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 100
    sort_by = x
    variable = frac
  []
[]

[Outputs]
  [console]
    type = Console
    execute_on = 'nonlinear linear'
  []
  [csv]
    type = CSV
    sync_times = '0.1 0.5 1'
    sync_only = true
  []
  interval = 10
[]
