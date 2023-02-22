[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  xmax = 1
[]

[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
  local_residual_normalization = true
[]

[Variables]
  [u][]
  [v]
    scaling = 1e-6
  []
[]

[Functions]
  [ramp]
    type = ParsedFunction
    expression = 'if(t < 5, t - 5, 0)'
  []
[]

[Kernels]
  [u_dt]
    type = TimeDerivative
    variable = u
    reference_residual_tags = 'ref'
  []
  [u_diff]
    type = MatDiffusion
    variable = u
    diffusivity = 1
  []
  [u_coupled_rx]
    type = CoupledForce
    variable = u
    v = v
    coef = 1e-6
    reference_residual_tags = 'ref'
  []

  [v_dt]
    type = TimeDerivative
    variable = v
    reference_residual_tags = 'ref'
  []
  [v_diff]
    type = MatDiffusion
    variable = v
    diffusivity = 1
  []
  [v_force]
    type = BodyForce
    variable = v
    value = 1e6
    function = ramp
    reference_residual_tags = 'ref'
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
  [v_avg]
    type = ElementAverageValue
    variable = v
  []
  [timestep]
    type = TimePostprocessor
  []
  [v_exact]
    type = ParsedPostprocessor
    pp_names = 'timestep'
    function = 't := if(timestep > 5, 5, timestep); (t^2 - t * 9) / 2 * 1e6'
  []
[]


[Executioner]
  type = Transient
  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 10
  nl_rel_tol = 1e-06
[]

[Outputs]
  csv = true
[]
