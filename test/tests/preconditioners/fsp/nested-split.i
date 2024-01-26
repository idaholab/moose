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
    topsplit = 'by_var'
    [by_var]
      splitting = 'u v'
      splitting_type  = multiplicative
      petsc_options_iname = '-ksp_type'
      petsc_options_value = 'fgmres'
    []
      [u]
        vars = 'u'
        splitting = 'u_diri u_bulk'
        splitting_type = multiplicative
        petsc_options_iname = '-ksp_type'
        petsc_options_value = 'fgmres'
      []
        [u_diri]
          vars = 'u'
          petsc_options = '-ksp_view_pmat'
          sides = 'left right'
        []
        [u_bulk]
          vars = 'u'
          petsc_options = '-ksp_view_pmat'
          petsc_options_iname = '-ksp_type'
          petsc_options_value = 'cg'
          unsides = 'left right'
        []
      [v]
        vars = 'v'
        splitting = 'v_diri v_bulk'
        splitting_type = multiplicative
        petsc_options_iname = '-ksp_type'
        petsc_options_value = 'fgmres'
      []
        [v_diri]
          vars = 'v'
          petsc_options = '-ksp_view_pmat'
          sides = 'left right'
        []
        [v_bulk]
          vars = 'v'
          petsc_options = '-ksp_view_pmat'
          petsc_options_iname = '-ksp_type'
          petsc_options_value = 'cg'
          unsides = 'left right'
        []
  []
[]

[Outputs]
  exodus = true
[]
