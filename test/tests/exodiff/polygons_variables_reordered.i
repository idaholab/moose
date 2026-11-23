# Same mesh and fields as polygons_variables.i, with the pentagon written before the
# quadrilateral. This is compared against that input's gold file, so matching the two by element
# location has to pair elements of differing node counts across the block, which only works if
# each element's connectivity is found at its own offset.
[Mesh]
  [pentagon]
    type = ElementGenerator
    nodal_positions = '2 0 0
                       3 0 0
                       3.5 1 0
                       2.5 1.5 0
                       1.5 1 0'

    element_connectivity = '0 1 2 3 4'
    elem_type = "C0POLYGON"
    subdomain_name = 'polygons'
  []
  [quad]
    type = ElementGenerator
    input = 'pentagon'
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "C0POLYGON"
    subdomain_name = 'polygons'
  []
[]

[AuxVariables]
  [nodal_var]
    order = FIRST
    family = LAGRANGE
  []
  [elemental_var]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [nodal_var]
    type = FunctionAux
    variable = nodal_var
    function = 'x + 2*y'
  []
  [elemental_var]
    type = FunctionAux
    variable = elemental_var
    function = '10*x'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
