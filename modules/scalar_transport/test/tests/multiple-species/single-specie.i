Krtt=0
Kdt2=1
Pt2_left=1
Pt2_right=0
d_t=1
l=1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = ${l}
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = ref
[]

[Variables]
  [t][]
[]

[Kernels]
  [time_t]
    type = TimeDerivative
    variable = t
    extra_vector_tags = ref
  []
  [diff_t]
    type = MatDiffusion
    variable = t
    diffusivity = ${d_t}
    extra_vector_tags = ref
  []
[]

[BCs]
  [tt_recombination]
    type = BinaryRecombinationBC
    variable = t
    v = t
    Kr = ${Krtt}
    boundary = 'left right'
  []
  [t_from_t2_left]
    type = DissociationFluxBC
    variable = t
    v = ${Pt2_left} # Partial pressure of T2
    Kd = ${Kdt2}
    boundary = left
  []
  [t_from_t2_right]
    type = DissociationFluxBC
    variable = t
    v = ${Pt2_right} # Partial pressure of T2
    Kd = ${Kdt2}
    boundary = right
  []
[]

[Postprocessors]
  [downstream_t_flux]
    type = SideFluxAverage
    variable = t
    boundary = right
    diffusivity = ${d_t}
  []
  [downstream_t_conc]
    type = SideAverageValue
    variable = t
    boundary = right
    outputs = 'none'
  []
  [upstream_t_conc]
    type = SideAverageValue
    variable = t
    boundary = left
    outputs = 'none'
  []
  [difference]
    type = DifferencePostprocessor
    value1 = upstream_t_conc
    value2 = downstream_t_conc
    outputs = 'none'
  []
  [domain_averaged_flux]
    type = ScalePostprocessor
    scaling_factor = ${fparse d_t / l}
    value = difference
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 40
  steady_state_detection = true
  dt = .1
[]

[Outputs]
  exodus = true
[]
