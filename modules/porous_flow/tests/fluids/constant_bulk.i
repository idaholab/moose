# Test the density calculated by the constant bulk Material
# Pressure 10 MPa
# Bulk modulus 2e8
# Density0 = 1000
# Density should equal 1051.27 kg/m^3

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Variables]
  [./pp]
    initial_condition = 10e6
  [../]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = pp
  [../]
[]

[Materials]
  [./ppss]
    type = PorousFlowMaterial1PhaseP_VG
    porepressure = pp
    al = 1
    m = 0.5
  [../]
  [./dens0]
    type = PorousFlowMaterialDensityConstBulk
    bulk_modulus = 2e8
    density0 = 1000
    phase = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Postprocessors]
  [./density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density0'
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = constant_bulk
  csv = true
[]
