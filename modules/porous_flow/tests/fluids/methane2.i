# Test the density and viscosity calculated by the methane Material
# Pressure 2 MPa
# Temperature 150C
# Water density should equal 9.1744 kg/m^3 (NIST webbook)
# Water viscosity should equal 1.5104e-05 Pa.s (NIST webbook)

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
    initial_condition = 2e6
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
    temperature = '150'
    al = 1
    m = 0.5
  [../]
  [./methane]
    type = PorousFlowMaterialMethane
    outputs = all
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
  [./viscosity]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity0'
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = methane2
  csv = true
[]
