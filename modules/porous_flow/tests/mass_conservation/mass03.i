# checking that the mass postprocessor correctly calculates the mass
# 1phase, 1component, constant porosity, with a constant fluid source
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]

[Variables]
  [./pp]
    initial_condition = -0.5
  [../]
[]


[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    component_index = 0
    variable = pp
  [../]
  [./source]
    type = BodyForce
    variable = pp
    value = 0.1 # kg/m^3/s
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
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMaterialMassFractionBuilder
  [../]
  [./dens0]
    type = PorousFlowMaterialDensityConstBulk
    density_P0 = 1
    bulk_modulus = 1
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowMaterialJoinerOld
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./porosity]
    type = PorousFlowMaterialPorosityConst
    porosity = 0.1
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
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres bjacobi 1E-12 1E-20 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 10
[]

[Outputs]
  execute_on = 'initial timestep_end'
  file_base = mass03
  csv = true
[]
