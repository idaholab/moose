[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
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
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = PostprocessorDirichletBC
    variable = u
    boundary = right
    postprocessor = 'num_coupling'
  []
[]

[Executioner]
  type = Steady
  coupling_min_its = 10
  coupling_max_its = 10
[]

[Postprocessors]
  [num_coupling]
    type = NumCouplingIterations
    execute_on = 'initial timestep_begin timestep_end'
  []
  [norm]
    type = ElementL2Norm
    variable = u
    execute_on = 'initial timestep_begin timestep_end'
  []
  [change_over_picard]
    type = ChangeOverPicardPostprocessor
    postprocessor = norm
    change_with_respect_to_initial = false
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = 'change_over_picard_previous'
  csv = true
[]
