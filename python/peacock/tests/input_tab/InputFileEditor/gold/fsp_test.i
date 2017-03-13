[BCs]
  active = 'left_u right_u left_v'
  [./left_u]
    boundary = '1'
    type = DirichletBC
    variable = u
    value = 0
  [../]
  [./right_u]
    boundary = '2'
    type = DirichletBC
    variable = u
    value = 100
  [../]
  [./left_v]
    boundary = '1'
    type = DirichletBC
    variable = v
    value = 0
  [../]
  [./right_v]
    boundary = '2'
    type = DirichletBC
    variable = v
    value = 0
  [../]
[]

[Executioner]
  # This is setup automatically in MOOSE (SetupPBPAction.C)
  # petsc_options = '-snes_mf_operator'
  # petsc_options_iname = '-pc_type'
  # petsc_options_value =  'asm'
  type = Steady
[]

[Kernels]
  active = 'diff_u conv_v diff_v'
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./conv_v]
    type = CoupledForce
    v = 'u'
    variable = v
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Outputs]
  exodus = true
  file_base = out
[]

[Preconditioning]
  active = 'FSP'
  [./FSP]
    # It is the starting point of splitting
    type = FSP
    topsplit = 'uv'    # uv should match the following block name
    [./uv]
      # Generally speaking, there are four types of splitting we could choose
      # <additive,multiplicative,symmetric_multiplicative,schur>
      # An approximate solution to the original system
      # | A_uu  A_uv | | u | _ |f_u|
      # |  0    A_vv | | v | - |f_v|
      # is obtained by solving the following subsystems
      # A_uu u = f_u and A_vv v = f_v
      # If splitting type is specified as schur, we may also want to set more options to
      # control how schur works using PETSc options
      # petsc_options_iname = '-pc_fieldsplit_schur_fact_type -pc_fieldsplit_schur_precondition'
      # petsc_options_value = 'full selfp'
      splitting = 'u v'      # u and v are the names of subsolvers
      splitting_type = additive
    [../]
    [./u]
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre preonly'
      vars = 'u'
    [../]
    [./v]
      # PETSc options for this subsolver
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre  preonly'
      vars = 'v'
    [../]
  [../]
[]

[Variables]
  active = 'u v'
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
  [./v]
    family = LAGRANGE
    order = FIRST
  [../]
[]

