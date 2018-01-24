###########################################################
# This test exercises the customer Preconditioner System.
# A Single Matrix Preconditioner is built using
# coupling specified by the user.
#
# @Requirement F1.40
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]

  [./p]
  [../]
  [./q]
  [../]
[]

# Single Matrix Preconditioner
[Preconditioning]
  [./SMP]
    type = SMP
    coupled_groups = 'u,v p,q'
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./conv_u]
    type = CoupledForce
    variable = u
    v = v
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]

  [./diff_p]
    type = Diffusion
    variable = p
  [../]
  [./conv_p]
    type = CoupledForce
    variable = p
    v = q
  [../]
  [./diff_q]
    type = Diffusion
    variable = q
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./bottom_v]
    type = DirichletBC
    variable = v
    boundary = 0
    value = 5
  [../]
  [./top_v]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 2
  [../]

  [./left_p]
    type = DirichletBC
    variable = p
    boundary = 1
    value = 2
  [../]
  [./bottom_q]
    type = DirichletBC
    variable = q
    boundary = 0
    value = 3
  [../]
  [./top_q]
    type = DirichletBC
    variable = q
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_max_its = 2
[]

[Outputs]
  exodus = true
[]
