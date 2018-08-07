# Test the Jacobian terms for the CoupledBEEquilibriumSub Kernel

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./a]
    order = FIRST
    family = LAGRANGE
  [../]
  [./b]
    order = FIRST
    family = LAGRANGE
  [../]
  [./pressure]
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
  [./b]
    type = RandomIC
    variable = b
    max = 1
    min = 0
  [../]
[]

[Kernels]
  [./diff]
    type = DarcyFluxPressure
    variable = pressure
  [../]
  [./diff_b]
    type = Diffusion
    variable = b
  [../]
  [./a]
    type = CoupledBEEquilibriumSub
    variable = a
    v = b
    log_k = 2
    weight = 2
    sto_v = 1.5
    sto_u = 2
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
  type = Transient
  solve_type = NEWTON
  end_time = 1
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
