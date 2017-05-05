# Trivial test of PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity
# Porosity = 0.1
# Solid density = 2500
# Fluid density = 1000
# Expected bulk density = 2500 * (1 - 0.1) + 1000 * 0.1 = 2350

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
[]

[GlobalParams]
  block = 0
  PorousFlowDictator = dictator
[]

[Modules]
  [./FluidProperties]
    [./simple_fluid]
      type = SimpleFluidProperties
      thermal_expansion = 0
      bulk_modulus = 2E9
      density0 = 1000
    [../]
  [../]
[]

[Variables]
  [./pp]
    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]
[]

[Functions]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = pp
  [../]
[]

[BCs]
  [./p]
    type = PresetBC
    variable = pp
    boundary = 'front back'
    value = 0
  [../]
[]

[AuxVariables]
  [./density]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./density]
    type = MaterialRealAux
    property = density
    variable = density
  [../]
[]

[Postprocessors]
  [./density]
    type = ElementalVariableValue
    elementid = 0
    variable = density
    execute_on = 'timestep_end'
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./ppss_qp]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
  [../]
  [./porosity_qp]
    type = PorousFlowPorosityConst
    porosity = 0.1
  [../]
  [./density]
    type = PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity
    rho_s = 2500
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = Newton
  type = Steady
  l_tol = 1E-5
  nl_abs_tol = 1E-4
  nl_rel_tol = 1E-8
  l_max_its = 200
  nl_max_its = 10
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
[]


[Outputs]
  file_base = GravDensity01
  csv = true
  execute_on = 'timestep_end'
[]
