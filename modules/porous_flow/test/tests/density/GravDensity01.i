# Trivial test of PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity
# Porosity = 0.1
# Solid density = 3
# Fluid density = 2
# Fluid bulk modulus = 4
# Fluid pressure = 0
# Bulk density: rho = 3 * (1 - 0.1) + 2 * 0.1 = 2.9
# Derivative wrt fluid pressure: d_rho / d_pp = d_rho / d_rho_f * d_rho_f / d_pp
#   = phi * rho_f / B
#   where rho_f = rho_0 * exp(pp / B) is fluid density, pp is fluid pressure, phi is
#   porosity and B is fluid bulk modulus
# With pp = 0, d_rho / d_pp = phi * rho_0 / B = 0.1 * 2 / 4 = 0.05

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = -1
  zmax = 0
  nx = 1
  ny = 1
  nz = 1
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0
    bulk_modulus = 4
    density0 = 2
  []
[]

[Variables]
  [pp]
    [InitialCondition]
      type = ConstantIC
      value = 0
    []
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = pp
  []
[]

[BCs]
  [p]
    type = DirichletBC
    variable = pp
    boundary = 'front back'
    value = 0
  []
[]

[AuxVariables]
  [density]
    order = CONSTANT
    family = MONOMIAL
  []
  [ddensity_dpp]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [density]
    type = MaterialRealAux
    property = density
    variable = density
  []
  [ddensity_dpp]
    type = MaterialStdVectorAux
    property = ddensity_dvar
    variable = ddensity_dpp
    index = 0
  []
[]

[Postprocessors]
  [density]
    type = ElementalVariableValue
    elementid = 0
    variable = density
    execute_on = 'timestep_end'
  []
  [ddensity_dpp]
    type = ElementalVariableValue
    elementid = 0
    variable = ddensity_dpp
    execute_on = 'timestep_end'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss_qp]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
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
  [density]
    type = PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity
    rho_s = 3
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  solve_type = Newton
  type = Steady
[]


[Outputs]
  file_base = GravDensity01
  csv = true
  execute_on = 'timestep_end'
[]
