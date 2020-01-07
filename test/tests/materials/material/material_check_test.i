[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4
    ny = 4
  []
  [./block_1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    top_right = '0.5 0.5 0'
    bottom_left = '0 0 0'
    block_id = 1
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./mat]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./mat]
    type = MaterialRealAux
    variable = mat
    property = prop
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 3
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    block = 1
    prop_names = prop
    prop_values = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  file_base = out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
