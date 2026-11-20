# HFEM on the displaced mesh with one refined element. The displacements are zero, so the solution
# matches the one on the reference mesh. On the coarse/fine faces, the interior lower-d element
# belongs to the coarse element while the internal side loop visits the fine one.
[Problem]
  use_hash_table_matrix_assembly = true
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
    subdomain_ids = '0 1 0 0'
  []
  [refine]
    type = RefineBlockGenerator
    input = square
    block = 1
    refinement = 1
  []
  displacements = 'disp_x disp_y'
  build_all_side_lowerd_mesh = true
[]

[Variables]
  [u]
    order = THIRD
    family = MONOMIAL
    block = '0 1'
  []
  [lambda]
    order = FIRST
    family = MONOMIAL
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
[]

[AuxVariables]
  [disp_x]
    block = '0 1'
  []
  [disp_y]
    block = '0 1'
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = '1'
    block = '0 1'
    use_displaced_mesh = true
  []
  [source]
    type = BodyForce
    variable = u
    value = '1'
    block = '0 1'
    use_displaced_mesh = true
  []
[]

[DGKernels]
  [surface]
    type = HFEMDiffusion
    variable = u
    lowerd_variable = lambda
    use_displaced_mesh = true
  []
[]

[BCs]
  [all]
    type = VacuumBC
    boundary = 'left right top bottom'
    variable = u
    use_displaced_mesh = true
  []
[]

[Postprocessors]
  [intu]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = '0 1'
  []
  [lambdanorm]
    type = ElementL2Norm
    variable = lambda
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       basic                 mumps'
[]

[Outputs]
  csv = true
[]
