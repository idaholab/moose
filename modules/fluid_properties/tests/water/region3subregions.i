# Test the Water97FluidProperties module in region 3 by recovering the values
# in Tables 5 and 13 of Revised Supplementary Release on Backward Equations for Specific
# Volume as a Function of Pressure and Temperature v(p,T) for Region 3 of the
# IAPWS Industrial Formulation 1997 for the Thermodynamic Properties of Water and Steam

[Mesh]
  type = FileMesh
  file = region3mesh.e
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    order = CONSTANT
    family = MONOMIAL
    initial_from_file_var = pressure
  [../]
  [./temperature]
    order = CONSTANT
    family = MONOMIAL
    initial_from_file_var = temperature
  [../]
  [./rho]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./v]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./rho]
    type = MaterialRealAux
    variable = rho
    property = density
  [../]
  [./v]
    type = ParsedAux
    args = rho
    function = 1/rho
    variable = v
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
  [./v0]
    type = ElementalVariableValue
    variable = v
    elementid = 0
  [../]
  [./v1]
    type = ElementalVariableValue
    variable = v
    elementid = 1
  [../]
  [./v2]
    type = ElementalVariableValue
    variable = v
    elementid = 2
  [../]
  [./v3]
    type = ElementalVariableValue
    variable = v
    elementid = 3
  [../]
  [./v4]
    type = ElementalVariableValue
    variable = v
    elementid = 4
  [../]
  [./v5]
    type = ElementalVariableValue
    variable = v
    elementid = 5
  [../]
  [./v6]
    type = ElementalVariableValue
    variable = v
    elementid = 6
  [../]
  [./v7]
    type = ElementalVariableValue
    variable = v
    elementid = 7
  [../]
  [./v8]
    type = ElementalVariableValue
    variable = v
    elementid = 8
  [../]
  [./v9]
    type = ElementalVariableValue
    variable = v
    elementid = 9
  [../]
  [./v10]
    type = ElementalVariableValue
    variable = v
    elementid = 10
  [../]
  [./v11]
    type = ElementalVariableValue
    variable = v
    elementid = 11
  [../]
  [./v12]
    type = ElementalVariableValue
    variable = v
    elementid = 12
  [../]
  [./v13]
    type = ElementalVariableValue
    variable = v
    elementid = 13
  [../]
  [./v14]
    type = ElementalVariableValue
    variable = v
    elementid = 14
  [../]
  [./v15]
    type = ElementalVariableValue
    variable = v
    elementid = 15
  [../]
  [./v16]
    type = ElementalVariableValue
    variable = v
    elementid = 16
  [../]
  [./v17]
    type = ElementalVariableValue
    variable = v
    elementid = 17
  [../]
  [./v18]
    type = ElementalVariableValue
    variable = v
    elementid = 18
  [../]
  [./v19]
    type = ElementalVariableValue
    variable = v
    elementid = 19
  [../]
  [./v20]
    type = ElementalVariableValue
    variable = v
    elementid = 20
  [../]
  [./v21]
    type = ElementalVariableValue
    variable = v
    elementid = 21
  [../]
  [./v22]
    type = ElementalVariableValue
    variable = v
    elementid = 22
  [../]
  [./v23]
    type = ElementalVariableValue
    variable = v
    elementid = 23
  [../]
  [./v24]
    type = ElementalVariableValue
    variable = v
    elementid = 24
  [../]
  [./v25]
    type = ElementalVariableValue
    variable = v
    elementid = 25
  [../]
  [./v26]
    type = ElementalVariableValue
    variable = v
    elementid = 26
  [../]
  [./v27]
    type = ElementalVariableValue
    variable = v
    elementid = 27
  [../]
  [./v28]
    type = ElementalVariableValue
    variable = v
    elementid = 28
  [../]
  [./v29]
    type = ElementalVariableValue
    variable = v
    elementid = 29
  [../]
  [./v30]
    type = ElementalVariableValue
    variable = v
    elementid = 30
  [../]
  [./v31]
    type = ElementalVariableValue
    variable = v
    elementid = 31
  [../]
  [./v32]
    type = ElementalVariableValue
    variable = v
    elementid = 32
  [../]
  [./v33]
    type = ElementalVariableValue
    variable = v
    elementid = 33
  [../]
  [./v34]
    type = ElementalVariableValue
    variable = v
    elementid = 34
  [../]
  [./v35]
    type = ElementalVariableValue
    variable = v
    elementid = 35
  [../]
  [./v36]
    type = ElementalVariableValue
    variable = v
    elementid = 36
  [../]
  [./v37]
    type = ElementalVariableValue
    variable = v
    elementid = 37
  [../]
  [./v38]
    type = ElementalVariableValue
    variable = v
    elementid = 38
  [../]
  [./v39]
    type = ElementalVariableValue
    variable = v
    elementid = 39
  [../]
  [./v40]
    type = ElementalVariableValue
    variable = v
    elementid = 40
  [../]
  [./v41]
    type = ElementalVariableValue
    variable = v
    elementid = 41
  [../]
  [./v42]
    type = ElementalVariableValue
    variable = v
    elementid = 42
  [../]
  [./v43]
    type = ElementalVariableValue
    variable = v
    elementid = 43
  [../]
  [./v44]
    type = ElementalVariableValue
    variable = v
    elementid = 44
  [../]
  [./v45]
    type = ElementalVariableValue
    variable = v
    elementid = 45
  [../]
  [./v46]
    type = ElementalVariableValue
    variable = v
    elementid = 46
  [../]
  [./v47]
    type = ElementalVariableValue
    variable = v
    elementid = 47
  [../]
  [./v48]
    type = ElementalVariableValue
    variable = v
    elementid = 48
  [../]
  [./v49]
    type = ElementalVariableValue
    variable = v
    elementid = 49
  [../]
  [./v50]
    type = ElementalVariableValue
    variable = v
    elementid = 50
  [../]
  [./v51]
    type = ElementalVariableValue
    variable = v
    elementid = 51
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
