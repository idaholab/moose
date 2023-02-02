# Master mesh and sub mesh are same with 4x4 quad8 elements.
# parent mesh has top boundary fixed at u=2 and bottom fixed at u=0
# sub mesh has top boundary fixed at u = 0 and bottom fixed at u=1
# The u variable at right boundary of sub mesh is transferred to
# from_sub variable of parent mesh at left boundary

# Result is from_sub at left boundary has linearly increasing value starting
# with 0 at top and ending with 1 at bottom. from_sub is zero everywhere else
# in the parent mesh.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  elem_type = QUAD8
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[AuxVariables]
  [./from_sub]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 2.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0.0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = boundary_toparent_sub.i
  [../]
[]

[Transfers]
  [./from_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    from_boundaries = 'right'
    to_boundaries = 'left'
    variable = from_sub
  [../]
[]
