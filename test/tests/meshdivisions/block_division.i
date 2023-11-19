[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    dz = '0.4 0.5 0.6 0.7'
    ix = '2 1 1'
    iy = '2 3'
    iz = '1 1 1 1'
    subdomain_id = '0 1 1
                    2 2 2

                    3 4 4
                    5 5 5

                    0 1 1
                    2 2 2

                    3 4 4
                    5 5 5
                    '
  []
[]

[MeshDivisions]
  [block_div]
    type = SubdomainsDivision
  []
[]

[AuxVariables]
  [blocks]
    family = MONOMIAL
    order = CONSTANT
  []
  [div]
    family = MONOMIAL
    order = CONSTANT
  []
  [diff]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mesh_div]
    type = MeshDivisionAux
    variable = div
    mesh_division = 'block_div'
  []
  [subdomains]
    type = FunctorAux
    functor = 'blocks_fmat'
    variable = 'blocks'
  []
  [difference]
    type = ParsedAux
    variable = diff
    expression = 'blocks - div'
    coupled_variables = 'blocks div'
  []
[]

[FunctorMaterials]
  [fmat]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = 'blocks_fmat'
    subdomain_to_prop_value = '0 0 1 1 2 2 3 3 4 4 5 5'
  []
[]

[Postprocessors]
  [min_diff]
    type = ElementExtremeValue
    variable = diff
    value_type = 'min'
  []
  [max_diff]
    type = ElementExtremeValue
    variable = diff
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
