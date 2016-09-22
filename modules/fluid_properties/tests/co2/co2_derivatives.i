# Test CO2FluidProperties derivatives using the FluidPropertiesDerivativeTestMaterial
# This calculates derivates of density, internal energy and enthalpy wrt both
# pressure and temperature, as well as finite difference derivatives for each of these
# These are saved as elemental auxkernels that can be compared in the Exodus output

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 1
  xmax = 3
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./temperature]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./pic]
    type = ParsedFunction
    value = 'if(x<1,1e6, if(x<2, 5e6, 10e6))'
  [../]
  [./tic]
    type = ParsedFunction
    value = 'if(x<1,280, if(x<2, 350, 500))'
  [../]
[]

[ICs]
  [./p_ic]
    type = FunctionIC
    function = pic
    variable = pressure
  [../]
  [./t_ic]
    type = FunctionIC
    function = tic
    variable = temperature
  [../]
[]

[Modules]
  [./FluidProperties]
    [./co2]
      type = CO2FluidProperties
    [../]
  []
[]

[Materials]
  [./fp_mat]
    type = FluidPropertiesDerivativeTestMaterial
    pressure = pressure
    temperature = temperature
    fp = co2
    outputs = exodus
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
