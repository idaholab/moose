# Test MethaneFluidProperties derivatives

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
    initial_condition = 10.0e6
  [../]
  [./temperature]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 350
  [../]
[]

[Modules]
  [./FluidProperties]
    [./methane]
      type = MethaneFluidProperties
    [../]
  []
[]

[Materials]
  [./fp_mat]
    type = FluidPropertiesDerivativeTestMaterial
    pressure = pressure
    temperature = temperature
    fp = methane
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
