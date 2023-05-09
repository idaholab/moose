Krht=1
Krtt=1
Krhh=1
Kdh2=1
Kdt2=1
Kdht=1
Ph2_left=1
Pt2_left=2
Pht_left=3
Ph2_right=0
Pt2_right=0
Pht_right=0
d_h=1
d_t=1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = ref
[]

[Variables]
  [h][]
  [t][]
[]

[Kernels]
  [time_h]
    type = TimeDerivative
    variable = h
    extra_vector_tags = ref
  []
  [diff_h]
    type = MatDiffusion
    variable = h
    diffusivity = ${d_h}
    extra_vector_tags = ref
  []
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
  [ht_h_left]
    type = BinaryRecombinationBC
    variable = h
    v = t
    Kr = ${Krht}
    boundary = left
  []
  [ht_t_left]
    type = BinaryRecombinationBC
    variable = t
    v = h
    Kr = ${Krht}
    boundary = left
  []
  [hh_left]
    type = BinaryRecombinationBC
    variable = h
    v = h
    Kr = ${Krhh}
    boundary = left
  []
  [tt_left]
    type = BinaryRecombinationBC
    variable = t
    v = t
    Kr = ${Krtt}
    boundary = left
  []
  [h_from_h2_left]
    type = DissociationFluxBC
    variable = h
    v = ${Ph2_left} # Partial pressure of H2
    Kd = ${Kdh2}
    boundary = left
  []
  [t_from_t2_left]
    type = DissociationFluxBC
    variable = t
    v = ${Pt2_left} # Partial pressure of T2
    Kd = ${Kdt2}
    boundary = left
  []
  [h_from_ht_left]
    type = DissociationFluxBC
    variable = h
    v = ${Pht_left} # Partial pressure of HT
    Kd = ${Kdht}
    boundary = left
  []
  [t_from_ht_left]
    type = DissociationFluxBC
    variable = t
    v = ${Pht_left} # Partial pressure of HT
    Kd = ${Kdht}
    boundary = left
  []
  [ht_h_right]
    type = BinaryRecombinationBC
    variable = h
    v = t
    Kr = ${Krht}
    boundary = right
  []
  [ht_t_right]
    type = BinaryRecombinationBC
    variable = t
    v = h
    Kr = ${Krht}
    boundary = right
  []
  [hh_right]
    type = BinaryRecombinationBC
    variable = h
    v = h
    Kr = ${Krhh}
    boundary = right
  []
  [tt_right]
    type = BinaryRecombinationBC
    variable = t
    v = t
    Kr = ${Krtt}
    boundary = right
  []
  [h_from_h2_right]
    type = DissociationFluxBC
    variable = h
    v = ${Ph2_right} # Partial pressure of H2
    Kd = ${Kdh2}
    boundary = right
  []
  [t_from_t2_right]
    type = DissociationFluxBC
    variable = t
    v = ${Pt2_right} # Partial pressure of T2
    Kd = ${Kdt2}
    boundary = right
  []
  [h_from_ht_right]
    type = DissociationFluxBC
    variable = h
    v = ${Pht_right} # Partial pressure of HT
    Kd = ${Kdht}
    boundary = right
  []
  [t_from_ht_right]
    type = DissociationFluxBC
    variable = t
    v = ${Pht_right} # Partial pressure of HT
    Kd = ${Kdht}
    boundary = right
  []
[]

[Postprocessors]
  [downstream_h_flux]
    type = SideFluxAverage
    variable = h
    boundary = right
    diffusivity = ${d_h}
  []
  [downstream_t_flux]
    type = SideFluxAverage
    variable = t
    boundary = right
    diffusivity = ${d_t}
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1000
  steady_state_detection = true
  steady_state_tolerance = 3e-08
  dt = .1
[]

[Outputs]
  exodus = true
[]
