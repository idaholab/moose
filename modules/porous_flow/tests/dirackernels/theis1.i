# Theis problem: Flow to single sink
# SinglePhase
# Cartesian mesh with logarithmic distribution in x and y.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  bias_x = 1.1
  bias_y = 1.1
  ymax = 100
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]

[Variables]
  [./pp]
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    component_index = 0
    variable = pp
  [../]
  [./flux]
    type = PorousFlowAdvectiveFlux
    variable = pp
    gravity = '0 0 0'
    component_index = 0
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
  [./ppss]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1E-7
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density
    include_old = true
  [../]
  [./dens_all_at_quadpoints]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_qps = true
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-14 0 0 0 1E-14 0 0 0 1E-14'
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityCorey
    n_j = 0
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity
  [../]
[]

[Postprocessors]
  [./porepressure]
    type = PointValue
    point = '0 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  [../]
  [./total_mass]
    type = PorousFlowFluidMass
    variable = pp
    execute_on = 'initial timestep_end'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 7E1
  end_time = 1E3
  nl_abs_tol = 1e-10
[]

[Outputs]
  print_perf_log = true
  file_base = theis1
  csv = true
  execute_on = 'final'
  [./con]
    output_linear = true
    type = Console
  [../]
[]

[ICs]
  [./PressureIC]
    variable = pp
    type = ConstantIC
    value = 20e6
  [../]
[]

[DiracKernels]
  [./sink]
    type = PorousFlowSquarePulsePointSource
    end_time = 1000
    point = '0 0 0'
    mass_flux = -0.04
    variable = pp
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = pp
    value = 20e6
    boundary = right
  [../]
  [./top]
    type = DirichletBC
    variable = pp
    value = 20e6
    boundary = top
  [../]
[]

[VectorPostprocessors]
  [./pressure]
    type = SideValueSampler
    variable = pp
    sort_by = x
    execute_on = timestep_end
    boundary = bottom
  [../]
[]
