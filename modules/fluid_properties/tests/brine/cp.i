# Test BrineFluidProperties calculations of isobaric heat capacity cp
#
# It is difficult to compare with experimental data, so instead we recreate
# the data presented in Figure 12 of Driesner, "The system H2o-NaCl. Part II:
# Correlations for molar volume, enthalpy, and isobaric heat capacity from
# 0 to 1000 C, 1 to 500 bar, and 0 to 1 Xnacl", Geochimica et Cosmochimica
# Acta 71, 4902-4919 (2007).
#
# Pressure = 179 bar (17.9 Mpa)
# Temperature from 50 C to 350 C (323.15 K to 623.15 K)
# NaCl molality 0.2124 mol/kg (equivalent to a mass fraction 0.01226 kg/kg)

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 1
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 17.9e6
  [../]
  [./temperature]
    family = LAGRANGE
    order = FIRST
  [../]
  [./xnacl]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0.01226
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = '323.15+300*x'
  [../]
[]

[ICs]
  [./t_ic]
    type = FunctionIC
    function = tic
    variable = temperature
  [../]
[]

[Modules]
  [./FluidProperties]
    [./brine]
      type = BrineFluidProperties
    [../]
  [../]
[]

[Materials]
  [./fp_mat]
    type = BrineFluidPropertiesTestMaterial
    pressure = pressure
    temperature = temperature
    xnacl = xnacl
    fp = brine
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[VectorPostprocessors]
  [./vpp]
    type = LineMaterialRealSampler
    start = '0 0 0'
    end = '1 0 0'
    property = cp
    sort_by = id
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
