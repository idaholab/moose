[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 1
  xmin = -1000
  xmax = 0
  ymin = 0
  ymax = 0.05
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = pressure
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 10
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 10
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_BW
    at_nodes = true
    porepressure = pressure
    Sn = 0.0
    Ss = 1.0
    C = 1.5
    las = 2
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP_BW
    porepressure = pressure
    Sn = 0.0
    Ss = 1.0
    C = 1.5
    las = 2
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityBW
    at_nodes = true
    Sn = 0.0
    Ss = 1.0
    Kn = 0
    Ks = 1
    C = 1.5
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.25
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0  0 1 0  0 0 1'
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 4
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
[]



[Variables]
  [./pressure]
    initial_condition = -1E-4
  [../]
[]


[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pressure
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pressure
    gravity = '-0.1 0 0'
  [../]
[]


[AuxVariables]
  [./SWater]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]


[AuxKernels]
  [./SWater]
    type = MaterialStdVectorAux
    property = PorousFlow_saturation_qp
    index = 0
    variable = SWater
  [../]
[]


[BCs]
  [./base]
    type = DirichletBC
    boundary = 'left'
    value = -1E-4
    variable = pressure
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-10      1E-10      10000'
  [../]
[]

[VectorPostprocessors]
  [./swater]
    type = LineValueSampler
    variable = SWater
    start_point = '-1000 0 0'
    end_point = '0 0 0'
    sort_by = x
    num_points = 71
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options = '-snes_converged_reason'
  end_time = 100
  dt = 5
[]


[Outputs]
  file_base = wli02
  sync_times = '100 500 1000'
  [./exodus]
    type = Exodus
    sync_only = true
  [../]
  [./along_line]
    type = CSV
    sync_only = true
  [../]
[]
