[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 41
    ny = 41
  []
  [./image]
    input = gen
    type = ImageSubdomainGenerator
    file = kitten.png
    threshold = 100
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./conv_v]
    type = CoupledForce
    variable = v
    v = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  active = 'left_u left_v right_u'
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 100
  [../]
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  [../]
[]

[Problem]
  type = FEProblem
  material_coverage_check = false
  kernel_coverage_check = false
[]

[Executioner]
  # This is setup automatically in MOOSE (SetupPBPAction.C)
  # petsc_options = '-snes_mf_operator'
  # petsc_options_iname = '-pc_type'
  # petsc_options_value =  'asm'
  type = Steady
[]

[Preconditioning]
  [./FSP]
    # It is the starting point of splitting
    type = FSP
    topsplit = 'uv' # 'uv'
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
      splitting = 'u v' # 'u' and 'v'
      splitting_type = additive
    [../]
    [./u]
      # PETSc options for this subsolver
      # A prefix will be applied, so just put the options for this subsolver only
      vars = u
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre preonly'
    [../]
    [./v]
      # PETSc options for this subsolver
      vars = v
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre  preonly'
    [../]
  [../]
[]

[Outputs]
  file_base = kitten_out
  exodus = true
[]
