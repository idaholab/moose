[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
  []
[]

[Problem]
  verbose_restore = true
[]

[VectorPostprocessors]
  [const]
    type = ConstantVectorPostprocessor
    value = '0 0; 1 1'
    vector_names = 'restored not_restored'
  []
[]

[Postprocessors]
  [old_vpp]
    type = UseOldVectorPostprocessor
    vpp = const
    vector_name = restored
  []
[]

[Executioner]
  type = Transient
  end_time = 5
[]

[Problem]
  type = FailingProblem
  fail_steps = 5
[]
