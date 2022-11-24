# Test IdealGasFluidPropertiesFluidProperties using pressure and temperature
# Use values for Oxygen at 1 MPa and 350 K from NIST chemistry webook
#
# Input values:
# Cv = 669.8e J/kg/K
# Cp = 938.75 J/kg/K
# M = 31.9988e-3 kg/mol
#
# Expected output:
# density = 10.99591793 kg/m^3
# internal energy = 234.43e3 J/kg
# enthalpy = 328.5625e3 J/kg
# speed of sound = 357.0151605 m/s

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1e6
  [../]
  [./temperature]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 350
  [../]
  [./density]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./viscosity]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./cp]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./cv]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./internal_energy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./enthalpy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./entropy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./thermal_cond]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./c]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./density]
    type = MaterialRealAux
     variable = density
     property = density
  [../]
  [./viscosity]
    type = MaterialRealAux
     variable = viscosity
     property = viscosity
  [../]
  [./cp]
    type = MaterialRealAux
     variable = cp
     property = cp
  [../]
  [./cv]
    type = MaterialRealAux
     variable = cv
     property = cv
  [../]
  [./e]
    type = MaterialRealAux
     variable = internal_energy
     property = e
  [../]
  [./enthalpy]
    type = MaterialRealAux
     variable = enthalpy
     property = h
  [../]
  [./entropy]
    type = MaterialRealAux
     variable = entropy
     property = s
  [../]
  [./thermal_cond]
    type = MaterialRealAux
     variable = thermal_cond
     property = k
  [../]
  [./c]
    type = MaterialRealAux
     variable = c
     property = c
  [../]
[]

[FluidProperties]
  [./idealgas]
    type = IdealGasFluidProperties
    gamma = 1.401537772469394
    molar_mass = 0.0319988
  [../]
[]

[Materials]
  [./fp_mat]
    type = FluidPropertiesMaterialPT
    pressure = pressure
    temperature = temperature
    fp = idealgas
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

[Outputs]
  exodus = true
[]
