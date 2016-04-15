# Test of new module with single phase.
# investigating pressure pulse in 1D with 1 phase


[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]
  
[Variables]
  [./pp]
    initial_condition = 2E6  
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
    type = PorousFlowMaterial1PhaseP_VG
    porepressure = pp
    al = 1E-7
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMaterialMassFractionBuilder
  [../]
  [./dens0]
    type = PorousFlowMaterialDensityConstBulk
    density0 = 1
    bulk_modulus = 1E7
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowMaterialJoinerOld
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./dens_all_at_quadpoints]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_fluid_phase_density_qp
  [../]
  [./porosity]
    type = PorousFlowMaterialPorosityConst
    porosity = 0.1
  [../]
  [./permeability]
    type = PorousFlowMaterialPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  [../]
  [./relperm]
    type = PorousFlowMaterialRelativePermeabilityConst
  [../]
    [./visc0]
    type = PorousFlowMaterialViscosityConst
    viscosity = 1
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_viscosity
  [../]
[]
  
[BCs]
  [./left]
    type = DirichletBC
    boundary = left
    value = 3E6
    variable = pp
  [../]
[]

[AuxVariables]

[]





[Preconditioning]

[]

[Executioner]
  type = Transient
  solve_type = Newton
    dt = 0.1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = pressure_pulse_1d
  exodus = true
[]
