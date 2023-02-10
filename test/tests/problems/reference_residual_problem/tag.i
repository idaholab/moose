scale = 100000000

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
  [u][]
[]

[Functions]
  [ramp]
    type = PiecewiseLinear
    x = '0 10'
    y = '0 1'
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
    reference_residual_tags = 'ref'
  []
  [neg_force]
    type = BodyForce
    variable = u
    value = ${fparse -scale / 2}
    function = ramp
    reference_residual_tags = 'ref'
  []
  [force]
    type = BodyForce
    variable = u
    value = ${scale}
    function = ramp
    reference_residual_tags = 'ref'
  []
[]

[Postprocessors]
  [u]
    type = ElementAverageValue
    variable = u
  []
[]


[Executioner]
  type = Transient
  petsc_options = '-snes_converged_reason'
  num_steps = 20
[]

[Outputs]
  exodus = true
[]
