[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
    components = 2
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [source]
    type = ArrayBodyForce
    variable = u
    function = '1 x'
  []
[]

[BCs]
  [left]
    type = ArrayDirichletBC
    variable = u
    boundary = 'left right bottom top'
    values = '0 0'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [pp_sub0]
    type = TransientMultiApp
    input_files = pp_sub.i
    execute_on = timestep_end
    positions = '0.5 0.5 0 0.7 0.7 0'
  []
  [pp_sub1]
    type = TransientMultiApp
    input_files = pp_sub.i
    execute_on = timestep_end
    positions = '0.5 0.5 0 0.7 0.7 0'
  []
[]

[Transfers]
  [sample_pp_transfer0]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    to_multi_app = pp_sub0
    postprocessor = from_parent
    source_variable = u
    source_variable_component = 0
  []
  [sample_pp_transfer1]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    to_multi_app = pp_sub1
    postprocessor = from_parent
    source_variable = u
    source_variable_component = 1
  []
[]
