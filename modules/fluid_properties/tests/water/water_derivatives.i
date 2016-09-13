# Test Water97FluidProperties derivatives

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 1
  xmax = 4
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
    value = 'if(x<1,3e6, if(x<2, 30e6, if(x<3, 25.588e6, 0.5e6)))'
  [../]
  [./tic]
    type = ParsedFunction
    value = 'if(x<1,300, if(x<2, 700, if(x<3, 650, 1500)))'
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
    [./water]
      type = Water97FluidProperties
    [../]
  []
[]

[Materials]
  [./fp_mat]
    type = FluidPropertiesDerivativeTestMaterial
    pressure = pressure
    temperature = temperature
    fp = water
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
