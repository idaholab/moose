# Tests the Jacobian when equilibrium secondary species are present including density
# in flux calculation

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [a]
    order = FIRST
    family = LAGRANGE
  []
  [b]
    order = FIRST
    family = LAGRANGE
  []
  [pressure]
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  [pressure]
    type = RandomIC
    variable = pressure
    max = 5
    min = 1
  []
  [a]
    type = RandomIC
    variable = a
    max = 1
    min = 0
  []
  [b]
    type = RandomIC
    variable = b
    max = 1
    min = 0
  []
[]

[ReactionNetwork]
  [AqueousEquilibriumReactions]
    use_kokkos = true
    primary_species = 'a b'
    reactions = '2a = pa2     2
                 a + b = pab 2'
    secondary_species = 'pa2 pab'
    pressure = pressure
  []
[]

[Kernels]
  [a_ie]
    type = KokkosPrimaryTimeDerivative
    variable = a
  []
  [a_diff]
    type = KokkosPrimaryDiffusion
    variable = a
  []
  [a_conv]
    type = KokkosPrimaryConvection
    variable = a
    p = pressure
    gravity = '0 -10 0'
  []
  [b_ie]
    type = KokkosPrimaryTimeDerivative
    variable = b
  []
  [b_diff]
    type = KokkosPrimaryDiffusion
    variable = b
  []
  [b_conv]
    type = KokkosPrimaryConvection
    variable = b
    p = pressure
    gravity = '0 -10 0'
  []
  [pressure]
    type = KokkosDarcyFluxPressure
    variable = pressure
    gravity = '0 -10 0'
  []
[]

[Materials]
  [porous]
    type = KokkosGenericConstantMaterial
    prop_names = 'diffusivity conductivity porosity density'
    prop_values = '1e-4 1e-4 0.2 10'
  []
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
  [smp]
    type = SMP
    full = true
  []
[]
