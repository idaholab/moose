# Test the Jacobian terms for the PrimaryConvection Kernel

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
  [../]
  [./a]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./pressure]
    type = RandomIC
    variable = pressure
    min = 1
    max = 5
  [../]
  [./a]
    type = RandomIC
    variable = a
    max = 1
    min = 0
  [../]
[]

[Kernels]
  [./diff]
    type = DarcyFluxPressure
    variable = pressure
  [../]
  [./conv]
    type = PrimaryConvection
    variable = a
    p = pressure
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = 'diffusivity conductivity porosity'
    prop_values = '1e-4 1e-4 0.2'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  perf_graph = true
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
