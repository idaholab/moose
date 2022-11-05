[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    xmax = 2
    ny = 40
    ymax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0.5 0'
    top_right = '1.5 1.5 0'
    block_id = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'primary1_interface'
  []
[]

[Variables]
  [u]
    block = 0
  []
  [v]
    block = 1
  []
[]

[Kernels]
  [diff_u]
    type = MatDiffusion
    variable = u
    diffusivity = D
    block = 0
  []
  [diff_v]
    type = MatDiffusion
    variable = v
    diffusivity = D
    block = 1
  []
  [source_u]
    type = BodyForce
    variable = u
    value = 1
    block = 0
  []
  [source_v]
    type = BodyForce
    variable = v
    value = 1
    block = 1
  []
[]

[BCs]
  [u]
    type = VacuumBC
    variable = u
    boundary = 'left bottom right top'
  []
  [interface_bc]
    type = ADMatchedValueBC
    variable = v
    v = u
    boundary = primary1_interface
  []
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[InterfaceKernels]
  active = 'diffusion'
  [./diffusion]
    type = InterfaceDiffusion
    variable = v
    neighbor_var = u
    boundary = primary1_interface
    D = 'D'
    D_neighbor = 'D'
  [../]

  [./penalty]
    type = PenaltyInterfaceDiffusion
    variable = v
    neighbor_var = u
    boundary = primary1_interface
    penalty = 1e3
  [../]
[]

[Materials]
  [mat0]
    type = GenericConstantMaterial
    prop_names = 'D'
    prop_values = '1'
    block = 0
  []
  [mat1]
    type = GenericConstantMaterial
    prop_names = 'D'
    prop_values = '1'
    block = 1
  []
[]

[AuxVariables]
  [c][]
[]

[AuxKernels]
  [u]
    type = ParsedAux
    variable = c
    coupled_variables = 'u'
    expression = 'u'
    block = 0
  []
  [v]
    type = ParsedAux
    variable = c
    coupled_variables = 'v'
    expression = 'v'
    block = 1
  []
[]
