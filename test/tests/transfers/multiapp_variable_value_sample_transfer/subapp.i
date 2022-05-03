[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [dummy]
    initial_condition = -2.0
  []
[]
[Problem]
  kernel_coverage_check = false
[]

[AuxVariables]
[]

[Kernels]
[]

[BCs]
[]

[Postprocessors]
  [from_primary_pp]
    type = Receiver
    default = -3.0
  []
  [to_primary_pp]
    type = ScalePostprocessor
    scaling_factor = 1
    value = from_primary_pp
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  dt = 1.0

  nl_abs_tol = 1e-13
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = false
[]
