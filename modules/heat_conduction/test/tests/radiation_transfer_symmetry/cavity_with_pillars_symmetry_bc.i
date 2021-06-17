#
# inner_left: 8
# inner_top: 11
# inner_bottom: 10
# inner_front: 9
# back_2: 7
# obstruction: 6
#

[Mesh]
  [cartesian]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.4 0.5 0.5 0.5'
    dy = '0.5 0.75 0.5'
    dz = '1.5 0.5'
    subdomain_id = '
                    3 1 1 1
                    3 1 2 1
                    3 1 1 1

                    3 1 1 1
                    3 1 1 1
                    3 1 1 1
                    '
  []

  [add_obstruction]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 2
    paired_block = 1
    new_boundary = obstruction
    input = cartesian
  []

  [add_new_back]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(z) < 1e-10'
    included_subdomain_ids = '1'
    normal = '0 0 -1'
    new_sideset_name = back_2
    input = add_obstruction
  []

  [add_inner_left]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 3
    paired_block = 1
    new_boundary = inner_left
    input = add_new_back
  []

  [add_inner_front]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(z - 2) < 1e-10'
    included_subdomain_ids = '1'
    normal = '0 0 1'
    new_sideset_name = inner_front
    input = add_inner_left
  []

  [add_inner_bottom]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(y) < 1e-10'
    included_subdomain_ids = '1'
    normal = '0 -1 0'
    new_sideset_name = inner_bottom
    input = add_inner_front
  []

  [add_inner_top]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(y - 1.75) < 1e-10'
    included_subdomain_ids = '1'
    normal = '0 1 0'
    new_sideset_name = inner_top
    input = add_inner_bottom
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [temperature]
    block = '2 3'
    initial_condition = 300
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
    block = '2 3'
    diffusion_coefficient = 1
  []

  [source]
    type = BodyForce
    variable = temperature
    value = 1000
    block = '2'
  []
[]

[BCs]
  [convective]
    type = CoupledConvectiveHeatFluxBC
    variable = temperature
    T_infinity = 300
    htc = 50
    boundary = 'left'
  []
[]

[GrayDiffuseRadiation]
  [./cavity]
    boundary = '6 7 8 9 10 11'
    emissivity = '1 1 1 1 1 1'
    n_patches = '1 1 1 1 1 1'
    adiabatic_boundary = '7 9 10 11'
    symmetry_boundary = '2'
    partitioners = 'metis metis metis metis metis metis'
    temperature = temperature
    ray_tracing_face_order = SECOND
    normalize_view_factor = false
  [../]
[]

[Postprocessors]
  [Tpv]
    type = PointValue
    variable = temperature
    point = '0.3 0.5 0.5'
  []

  [volume]
    type = VolumePostprocessor
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
