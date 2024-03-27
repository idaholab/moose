[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 50
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = 'gen_mesh'
    combinatorial_geometry = 'x < 0.5'
    block_id = '1'
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
  []
[]

[LinearFVKernels]
  [reaction_1]
    type = LinearFVReaction
    variable = u
    coeff = 1.5
    block = 0
  []
  [reaction_2]
    type = LinearFVReaction
    variable = u
    coeff = 2.5
    block = 1
  []
  [source_1]
    type = LinearFVSource
    variable = u
    source_density = 3.5
    block = 0
  []
  [source_2]
    type = LinearFVSource
    variable = u
    source_density = 4.5
    block = 1
  []
[]

[Executioner]
  type = LinearPicardSteady
  linear_systems_to_solve = u_sys
[]

[Debug]
  show_execution_order = 'NONLINEAR'
[]
