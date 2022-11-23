###########################################################
# This is a test of the Transfer System. This test
# uses the Multiapp System to solve independent problems
# related geometrically. Solutions are then interpolated
# and transferred from a non-aligned domain.
#
# @Requirement F7.20
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  displacements = 'disp_x disp_y'
  # The MultiAppGeometricInterpolationTransfer object only works with ReplicatedMesh
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./from_sub]
  [../]
  [./elemental_from_sub]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./radial_from_sub]
  [../]
  [./radial_elemental_from_sub]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./disp_x]
    initial_condition = 0.2
  [../]
  [./disp_y]
  [../]
  [./displaced_target_from_sub]
  [../]
  [./displaced_source_from_sub]
  [../]
  [./nodal_from_sub_elemental]
  [../]
  [./elemental_from_sub_elemental]
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

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
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
    positions = '0 0 0 0.6 0 0'
    input_files = fromsub_sub.i
  [../]
[]

[Transfers]
  [./fromsub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = u
    variable = from_sub
  [../]
  [./elemental_fromsub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = u
    variable = elemental_from_sub
  [../]
  [./radial_fromsub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = u
    variable = radial_from_sub
    interp_type = radial_basis
  [../]
  [./radial_elemental_fromsub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = u
    variable = radial_elemental_from_sub
    interp_type = radial_basis
  [../]
  [./displaced_target_fromsub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = u
    variable = displaced_target_from_sub
    displaced_target_mesh = true
  [../]
  [./displaced_source_fromsub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = u
    variable = displaced_source_from_sub
    displaced_source_mesh = true
  [../]
  [./elemental_from_sub_elemental]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = elemental
    variable = elemental_from_sub_elemental
  [../]
  [./nodal_from_sub_elemental]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = elemental
    variable = nodal_from_sub_elemental
  [../]
[]
