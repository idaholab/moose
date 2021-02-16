[Mesh]
  [genmesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4
    ny = 4
  []
  [mesh0]
    type = SubdomainBoundingBoxGenerator
    input = genmesh
    block_id = 0
    location = INSIDE
    bottom_left = '0 0 0'
    top_right = '1 0.5 0'
  []
  [mesh01]
    type = SubdomainBoundingBoxGenerator
    input = mesh0
    block_id = 1
    location = INSIDE
    bottom_left = '0 0.5 0'
    top_right = '1 1 0'
  []
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
    type = MatDiffusionTest
    variable = u
    prop_name = matp
  [../]
[]

[AuxKernels]
  [./mat]
    type = MaterialRealAux
    variable = mat
    property = matp
    execute_on = timestep_end
  [../]
[]

[VectorPostprocessors]
  [./mat]
    type = LineMaterialRealSampler
    start = '0.125 0.375 0.0'
    end   = '0.875 0.375 0.0'
    property = matp
    sort_by = id
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
    type = MTBC
    variable = u
    boundary = 1
    grad = 8
    prop_name = matp
  [../]
[]

[Materials]
  [./mat]
    type = MTMaterial
    block = '0 1'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out
  csv = true
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
