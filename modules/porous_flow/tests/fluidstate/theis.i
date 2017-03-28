# Two phase Theis problem: Flow from single source using WaterNCG fluidstate.
# Constant rate injection 0.5 kg/s
# 1D cylindrical mesh
# Initially, system has only a liquid phase, until enough gas is injected
# to form a gas phase, in which case the system becomes two phase.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 50
  xmax = 2000
  bias_x = 1.1
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = Y
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[AuxVariables]
  [./saturation_gas]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./saturation_gas]
    type = PorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = timestep_end
  [../]
[]

[Variables]
  [./pgas]
    initial_condition = 20e6
  [../]
  [./z]
    initial_condition = 0
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pgas
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pgas
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = z
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = z
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Modules]
  [./FluidProperties]
    [./co2]
      type = CO2FluidProperties
    [../]
    [./water]
      type = Water97FluidProperties
    [../]
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./temperature_qp]
    type = PorousFlowTemperature
  [../]
  [./waterncg]
    type = PorousFlowFluidStateWaterNCG
    gas_porepressure = pgas
    z = z
    gas_fp = co2
    water_fp = water
    at_nodes = true
    temperature_unit = Celsius
  [../]
  [./waterncg_qp]
    type = PorousFlowFluidStateWaterNCG
    gas_porepressure = pgas
    z = z
    gas_fp = co2
    water_fp = water
    temperature_unit = Celsius
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.2
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  [../]
  [./relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 0
  [../]
  [./relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
[]

[BCs]
  [./rightwater]
    type = DirichletBC
    boundary = right
    value = 20e6
    variable = pgas
  [../]
[]

[DiracKernels]
  [./source]
    type = PorousFlowSquarePulsePointSource
    point = '0 0 0'
    mass_flux = 0.5
    variable = z
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-8       1E-10 20'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 3e4
  dtmax = 1e4
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    growth_factor = 2
  [../]
[]

[Postprocessors]
  [./pgas]
    type = PointValue
    point =  '4 0 0'
    variable = pgas
  [../]
  [./z]
    type = PointValue
    point = '4 0 0'
    variable = z
  [../]
  [./massgas]
    type = PorousFlowFluidMass
    fluid_component = 1
  [../]
[]

[Outputs]
  print_linear_residuals = false
  print_perf_log = true
  exodus = true
[]
