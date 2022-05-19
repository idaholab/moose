[Problem]
  boundary_restricted_elem_integrity_check = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
[]

[AuxVariables]
  [dummy][]
  [dummy2]
    family = MONOMIAL
    order = CONSTANT
    block = 1
  []
  [dummy3]
    family = MONOMIAL
    order = CONSTANT
    block = 0
  []
[]

[AuxKernels]
  active = 'bad'
  [bad]
    type = SelfAux
    variable = dummy
    v = v
    boundary = 'left'
  []
  [bad_elemental]
    type = SelfAux
    variable = dummy3
    v = dummy2
    boundary = 'left'
  []
[]

[Variables]
  [u]
    block = '0'
  []
  [v]
    block = '1'
  []
[]

[Kernels]
  [diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 0
  []
  [diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
  []
[]

[InterfaceKernels]
  active = 'interface'
  [interface]
    type = InterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    D = 'D'
    D_neighbor = 'D'
  []
  [penalty_interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
  []
[]

[BCs]
  active = 'left right middle'
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [bad]
    type = MatchedValueBC
    variable = u
    boundary = 'left'
    v = v
  []
  [bad_integrated]
    type = CoupledVarNeumannBC
    variable = u
    boundary = 'left'
    v = v
  []
  [right]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 0
  []
  [middle]
    type = MatchedValueBC
    variable = v
    boundary = 'primary0_interface'
    v = u
  []
[]

[Materials]
  [stateful]
    type = StatefulMaterial
    initial_diffusivity = 1
    boundary = primary0_interface
  []
  [block0]
    type = GenericConstantMaterial
    block = '0'
    prop_names = 'D'
    prop_values = '4'
  []
  [block1]
    type = GenericConstantMaterial
    block = '1'
    prop_names = 'D'
    prop_values = '2'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  active = ''
  [bad]
    type = NodalExtremeValue
    boundary = 'left'
    variable = v
  []
  [bad_side]
    type = SideDiffusiveFluxIntegral
    variable = v
    diffusivity = 1
    boundary = 'left'
  []
[]
