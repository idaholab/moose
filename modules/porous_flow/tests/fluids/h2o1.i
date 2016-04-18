# Test the density and viscosity calculated by the water Material
# Region 1 density
# Pressure 80 MPa
# Temperature 300K (26.85C)
# Water density should equal 1.0 / 0.971180894e–3 = 1029.7 kg/m^3 (IAPWS IF97)
# Water viscosity should equal 0.00085327 Pa.s (NIST webbook)

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
    initial_condition = 80e6
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
    temperature = '26.85'
    al = 1
    m = 0.5
  [../]
  [./dens0]
    type = PorousFlowMaterialWater
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
  file_base = h2o1
  csv = true
[]
