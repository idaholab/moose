[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u][]
  [v][]
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []

  [conv_v]
    type = CoupledForce
    variable = v
    v = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 100
  []

  [left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  []
[]

[Executioner]
  type = Steady
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'top'
    [top]
      splitting = 'u_diri rest'
      splitting_type  = multiplicative
      petsc_options_iname = '-ksp_type'
      petsc_options_value = 'fgmres'
    []
      [u_diri]
        vars = 'u'
        sides = 'left right'
      []
      [rest]
        unside_by_var_var_name = 'u u'
        unside_by_var_boundary_name = 'left right'
      []
  []
[]

[Outputs]
  exodus = true
[]
