# Test BrineFluidProperties calculations of enthalpy
#
# It is difficult to compare with experimental data, so instead we recreate
# the data presented in Figure 11 of Driesner, "The system H2o-NaCl. Part II:
# Correlations for molar volume, enthalpy, and isobaric heat capacity from
# 0 to 1000 C, 1 to 500 bar, and 0 to 1 Xnacl", Geochimica et Cosmochimica
# Acta 71, 4902-4919 (2007).
#
# Pressure = 100 bar (10 Mpa)
# Temperature = 300 C (573.15 K)
# NaCl mole fraction from 0 to 0.15 (mass fraction from 0 to 0.364)

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
    initial_condition = 10e6
  [../]
  [./temperature]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 573.15
  [../]
  [./xnacl]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[Functions]
  [./xic]
    type = ParsedFunction
    value = 0.364*x
  [../]
[]

[ICs]
  [./x_ic]
    type = FunctionIC
    function = xic
    variable = xnacl
  [../]
[]

[Modules]
  [./FluidProperties]
    [./brine]
      type = BrineFluidProperties
    [../]
  []
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
    property = enthalpy
    sort_by = id
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
