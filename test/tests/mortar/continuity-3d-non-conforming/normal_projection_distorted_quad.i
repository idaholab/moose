[Mesh]
  [secondary_element]
    type = ElementGenerator
    nodal_positions = '-0.11010383 -0.08283381 -0.2
                        2.54346587 -1.49423481 -0.2
                        2.18944321  1.44100457 -0.2
                        0.90378190  0.72419628 -0.2
                       -0.11010383 -0.08283381  0.0
                        2.54346587 -1.49423481  0.0
                        2.18944321  1.44100457  0.0
                        0.90378190  0.72419628  0.0'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
    subdomain_id = 1
    subdomain_name = secondary_volume
    create_sidesets = true
  []
  [primary_element]
    type = ElementGenerator
    nodal_positions = '-0.04959756 -0.13648029 0.1
                        2.85934719 -0.30198830 0.1
                        1.08993312  2.50493407 0.1
                        0.38486310  1.16516148 0.1
                       -0.04959756 -0.13648029 0.3
                        2.85934719 -0.30198830 0.3
                        1.08993312  2.50493407 0.3
                        0.38486310  1.16516148 0.3'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
    subdomain_id = 2
    subdomain_name = primary_volume
    create_sidesets = true
  []
  [primary_sides]
    type = RenameBoundaryGenerator
    input = primary_element
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '10 11 12 13 14 15'
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'secondary_element primary_sides'
  []
  [interface_boundaries]
    type = RenameBoundaryGenerator
    input = combined
    old_boundary = '5 10'
    new_boundary = 'secondary primary'
  []
  [secondary_subdomain]
    type = LowerDBlockFromSidesetGenerator
    input = interface_boundaries
    sidesets = secondary
    new_block_id = 11
    new_block_name = secondary
  []
  [primary_subdomain]
    type = LowerDBlockFromSidesetGenerator
    input = secondary_subdomain
    sidesets = primary
    new_block_id = 12
    new_block_name = primary
  []
[]

[Variables]
  [T]
    block = '1 2'
  []
  [lambda]
    block = secondary
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = T
    block = '1 2'
  []
[]

[Constraints]
  [mortar]
    type = EqualValueConstraint
    primary_boundary = primary
    secondary_boundary = secondary
    primary_subdomain = primary
    secondary_subdomain = secondary
    variable = lambda
    secondary_variable = T
    mortar_3d_qp_mapping = normal_projection
    triangulation = vertex
    segment_quadrature = SEVENTH
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
