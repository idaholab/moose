[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    nz = 0
    zmin = 0
    zmax = 0
    elem_type = QUAD4
  []

  [./subdomain_id]
    type = ElementSubdomainIDGenerator
    input = gmg
    subdomain_ids = '0 1 2 3 4
                     0 1 2 3 4
                     0 1 2 3 4
                     0 1 2 3 4
                     0 1 2 3 4'
  []

  [./boundary01]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain_id
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'boundary01'
  []

  [./boundary10]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary01
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'boundary10'
  []

  [./boundary12]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary10
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'boundary12'
  []

  [./boundary21]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary12
    primary_block = '2'
    paired_block = '1'
    new_boundary = 'boundary21'
  []

  [./boundary23]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary21
    primary_block = '2'
    paired_block = '3'
    new_boundary = 'boundary23'
  []

  [./boundary32]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary23
    primary_block = '3'
    paired_block = '2'
    new_boundary = 'boundary32'
  []

  [./boundary34]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary32
    primary_block = '3'
    paired_block = '4'
    new_boundary = 'boundary34'
  []

  [./boundary43]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary34
    primary_block = '4'
    paired_block = '3'
    new_boundary = 'boundary43'
  []

  uniform_refine = 3
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxVariables]
  [./fromparent]
  []
  [./fromparentelem]
    order = constant
    family = monomial
  [../]
[]

[BCs]
  [./left0]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]

  [./right0]
    type = DirichletBC
    variable = u
    boundary = boundary01
    value = 1
  [../]

  [./right1]
    type = DirichletBC
    variable = u
    boundary = boundary12
    value = 1
  [../]

  [./right2]
    type = DirichletBC
    variable = u
    boundary = boundary23
    value = 0
  [../]

  [./right3]
    type = DirichletBC
    variable = u
    boundary = boundary34
    value = 0
  [../]

  [./right4]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
