D0 = 1
D1 = 2
D2 = 6

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1.5 3 2'
    ix = '3 3 4'
    subdomain_id = '0 1 2'
  []
  [add_01]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'cmg'
    primary_block = '0'
    paired_block = '1'
    new_boundary = '0to1'
  []
  [add_12]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'add_01'
    primary_block = '1'
    paired_block = '2'
    new_boundary = '1to2'
  []
[]

[Variables]
  [T_solid]
    type = MooseVariableFVReal
  []
[]

[FVBCs]
  [right]
    type = FVDirichletBC
    variable = T_solid
    boundary = 'right'
    value = 1
  []
[]

[FVKernels]
  [diff1]
    type = FVDiffusion
    variable = T_solid
    coeff = ${D0}
    block = 0
  []
  [diff2]
    type = FVDiffusion
    variable = T_solid
    coeff = ${D1}
    block = 1
  []
  [diff3]
    type = FVDiffusion
    variable = T_solid
    coeff = ${D2}
    block = 2
  []
  [source]
    type = FVBodyForce
    variable = T_solid
    value = 1
    block = 1
  []
[]

[FVInterfaceKernels]
  [01]
    type = FVOneVarDiffusionInterface
    variable1 = T_solid
    subdomain1 = '0'
    subdomain2 = '1'
    coeff1 = ${D0}
    coeff2 = ${D1}
    boundary = '0to1'
  []
  [12]
    type = FVOneVarDiffusionInterface
    variable1 = T_solid
    subdomain1 = '1'
    subdomain2 = '2'
    coeff1 = ${D1}
    coeff2 = ${D2}
    boundary = '1to2'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

[VectorPostprocessors]
  [all_values]
    type = ElementValueSampler
    variable = T_solid
    sort_by = 'x'
  []
[]
