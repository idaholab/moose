[Mesh]
  [file]
    type = FileMeshGenerator
    file = test_subdomain_partitioner.e
  []

  [./Partitioner]
    type = LibmeshPartitioner
    partitioner = subdomain_partitioner
    blocks = '1 2 3 4; 1001 1002 1003 1004'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./proc_id]
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
  [./proc_id]
    type = ProcessorIDAux
    variable = proc_id
  [../]
[]


[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = subdomain_partitioner_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
