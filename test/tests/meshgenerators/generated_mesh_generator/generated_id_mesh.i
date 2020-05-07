[Mesh]
  [gen]
    type = GeneratedIDMeshGenerator
    dim = 2
    xmin = -15.24
    xmax = 15.24
    ymin = -15.24
    ymax = 15.24
    nx = 3
    ny = 3
    subdomain_id = '12 3 4 0 36 35 33 14 11'
    material_id = '7 5 2 3 1 6 1 4 4'
    depletion_id = '3 2 1 6 8 7 11 10 12'
    equivalence_id = '3 3 1 1 0 8 6 6 4'
  []
  uniform_refine = 1
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[AuxVariables]
  [matid]
    family = MONOMIAL
    order = CONSTANT
  []
  [equid]
    family = MONOMIAL
    order = CONSTANT
  []
  [depid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [matid]
    type = ElemExtraIDAux
    variable = matid
    extra_id_name = material_id
    execute_on = initial
  []
  [equid]
    type = ElemExtraIDAux
    variable = equid
    extra_id_name = equivalence_id
    execute_on = initial
  []
  [depid]
    type = ElemExtraIDAux
    variable = depid
    extra_id_name = depletion_id
    execute_on = initial
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
