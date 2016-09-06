# Test the Water97FluidProperties module calculation of the boundary between regions 2 and
# 3 by checking that the function b23p and b23t meet at the point T = 623.15 K and
# p = 16.5291643 MPa. From Revised Release on the IAPWS Industrial Formulation 1997 for the
# Thermodynamic Properties of Water and Steam

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 16.5291643e6
  [../]
  [./temperature]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 623.15
  [../]
  [./b23p]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./b23T]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./b23p]
    type = MaterialRealAux
    variable = b23p
    property = b23p
  [../]
  [./b23T]
    type = MaterialRealAux
    variable = b23T
    property = b23T
  [../]
[]

[Modules]
  [./FluidProperties]
    [./water]
      type = Water97FluidProperties
    [../]
  [../]
[]

[Materials]
  [./fp_mat]
    type = WaterFluidPropertiesTestMaterial
    pressure = pressure
    temperature = temperature
    b23 = true
    fp = water
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
[]

[Postprocessors]
  [./b23p]
    type = ElementalVariableValue
    variable = b23p
    elementid = 0
  [../]
  [./b23T]
    type = ElementalVariableValue
    variable = b23T
    elementid = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
