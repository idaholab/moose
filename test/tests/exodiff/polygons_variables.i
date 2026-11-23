[Mesh]
  # A quadrilateral and a pentagon in one block, so that the node counts vary within the Exodus
  # "NSIDED" block that holds them.
  [quad]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "C0POLYGON"
    subdomain_name = 'polygons'
  []
  [pentagon]
    type = ElementGenerator
    input = 'quad'
    nodal_positions = '2 0 0
                       3 0 0
                       3.5 1 0
                       2.5 1.5 0
                       1.5 1 0'

    element_connectivity = '0 1 2 3 4'
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
