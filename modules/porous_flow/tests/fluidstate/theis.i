# Two phase Theis problem: Flow from single source using WaterNCG fluidstate.
# Constant rate injection 0.5 kg/s
# 1D cylindrical mesh
# Initially, system has only a liquid phase, until enough gas is injected
# to form a gas phase, in which case the system becomes two phase.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmax = 2000
  bias_x = 1.05
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
    order = FIRST
    family = MONOMIAL
  [../]
  [./x1]
    order = FIRST
    family = MONOMIAL
  [../]
  [./y0]
    order = FIRST
    family = MONOMIAL
  [../]
  [./pgas2]
    order = FIRST
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
  [./x1]
    type = PorousFlowPropertyAux
    variable = x1
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = timestep_end
  [../]
  [./y0]
    type = PorousFlowPropertyAux
    variable = y0
    property = mass_fraction
    phase = 1
    fluid_component = 0
    execute_on = timestep_end
  [../]
  [./pgas2]
    type = PorousFlowPropertyAux
    variable = pgas2
    property = pressure
    phase = 1
    execute_on = timestep_end
  [../]
[]

[Variables]
  [./pgas]
    initial_condition = 20e6
  [../]
  [./zi]
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
    variable = zi
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = zi
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas zi'
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
    z = zi
    gas_fp = co2
    water_fp = water
    at_nodes = true
    temperature_unit = Celsius
    sat_lr = 0.1
  [../]
  [./waterncg_qp]
    type = PorousFlowFluidStateWaterNCG
    gas_porepressure = pgas
    z = zi
    gas_fp = co2
    water_fp = water
    temperature_unit = Celsius
    sat_lr = 0.1
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
    n = 2
    phase = 0
    s_res = 0.1
    sum_s_res = 0.1
  [../]
  [./relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 2
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
    mass_flux = 2
    variable = zi
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
  solve_type = NEWTON
  end_time = 1e5
  dtmax = 1e5
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    growth_factor = 1.5
  [../]
[]

[VectorPostprocessors]
  [./line]
    type = NodalValueSampler
    sort_by = x
    variable = 'pgas zi'
    execute_on = 'timestep_end'
  [../]
  [./line2]
    type = LineValueSampler
    sort_by = x
    start_point = '0 0 0'
    end_point = '1000 0 0'
    num_points = 1500
    variable = 'saturation_gas x1 pgas'
  [../]
[]

[Postprocessors]
  [./pgas]
    type = PointValue
    point =  '4 0 0'
    variable = pgas
  [../]
  [./sgas]
    type = PointValue
    point =  '4 0 0'
    variable = saturation_gas
  [../]
  [./zi]
    type = PointValue
    point = '4 0 0'
    variable = zi
  [../]
  [./massgas]
    type = PorousFlowFluidMass
    fluid_component = 1
  [../]
  [./x1]
    type = PointValue
    point =  '4 0 0'
    variable = x1
  [../]
  [./y0]
    type = PointValue
    point =  '4 0 0'
    variable = y0
  [../]
[]

[Outputs]
  print_linear_residuals = false
  print_perf_log = true
  [./csvout]
    type = CSV
    execute_on = timestep_end
    execute_vector_postprocessors_on = final
  [../]
[]
