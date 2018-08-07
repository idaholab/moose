# Tests the Jacobian when equilibrium secondary species are present including density
# in flux calculation

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
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
    max = 5
    min = 1
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

[ReactionNetwork]
  [./AqueousEquilibriumReactions]
    primary_species = 'a b'
    reactions = '2a = pa2     2
                 a + b = pab 2'
    secondary_species = 'pa2 pab'
    pressure = pressure
  [../]
[]

[Kernels]
  [./a_ie]
    type = PrimaryTimeDerivative
    variable = a
  [../]
  [./a_diff]
    type = PrimaryDiffusion
    variable = a
  [../]
  [./a_conv]
    type = PrimaryConvection
    variable = a
    p = pressure
    gravity = '0 -10 0'
  [../]
  [./b_ie]
    type = PrimaryTimeDerivative
    variable = b
  [../]
  [./b_diff]
    type = PrimaryDiffusion
    variable = b
  [../]
  [./b_conv]
    type = PrimaryConvection
    variable = b
    p = pressure
    gravity = '0 -10 0'
  [../]
  [./pressure]
    type = DarcyFluxPressure
    variable = pressure
    gravity = '0 -10 0'
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = 'diffusivity conductivity porosity density'
    prop_values = '1e-4 1e-4 0.2 10'
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
