[Mesh]
    [generate_mesh]
        type = GeneratedMeshGenerator
        dim = 2
        xmin = 0
        xmax = 10
        ymin = 0
        ymax = 10
        zmin = 0
        zmax = 10
        nx = 5
        ny = 5
        nz = 5
    []
[]

[AuxVariables]
    [in_mesh]
        order = CONSTANT
        family = MONOMIAL
    []
    [hierarchy]
        order = CONSTANT
        family = MONOMIAL
    []
[]

[AuxKernels]
  [hierarchy_aux_kernel]
    type=ElementHierarchyAux
    variable=hierarchy
  []

    [fill_up_data]
        type = FunctionAux
        variable = in_mesh
        function = "3*x*y+sin(x*y)-12*x*x"
        execute_on = 'TIMESTEP_BEGIN'
    []
[]

[Adaptivity]
    marker = error_fraction
    steps = 1
  [Indicators]
    [error]
      type = ValueJumpIndicator
      variable = in_mesh
    []
  []

  [Markers]
    [error_fraction]
      type = ErrorFractionMarker
      indicator = error
      refine = 0.2
      coarsen = 0.1
    []
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  solve = false
  dt = 1
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
