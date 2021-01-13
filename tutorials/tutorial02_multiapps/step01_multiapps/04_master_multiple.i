[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = BodyForce
    variable = u
    value = 1.
  []
  [td]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Transient
  end_time = 2
  dt = 1.

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    positions   = '0 0 0  1 0 0  2 0 0'
#    positions_file = 04_positions.txt
    input_files = '04_sub1_multiple.i'
#    input_files = '04_sub1_multiple.i  04_sub2_multiple.i 04_sub3_multiple.i'
#    output_in_position = true
  []
[]
