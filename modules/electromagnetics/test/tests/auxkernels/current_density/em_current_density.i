# This test is a modification of the vector_helmholtz.vector_kernels test
# to verify functionality of the current density auxkernel for the case of
# a vector field variable in electromagnetic mode.
# Manufactured solution: u = y * x_hat - x * y_hat

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = -1
    ymin = -1
    elem_type = QUAD9
  []
[]

[Variables]
  [u]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[AuxVariables]
  [J]
    family = NEDELEC_ONE
    order = FIRST
  []
[]


[Kernels]
  [curl_curl]
    type = CurlCurlField
    variable = u
  []
  [coeff]
    type = VectorFunctionReaction
    variable = u
  []
  [rhs]
    type = VectorBodyForce
    variable = u
    function_x = 'y'
    function_y = '-x'
  []
[]

[BCs]
  [sides]
    type = VectorCurlPenaltyDirichletBC
    variable = u
    function_x = 'y'
    function_y = '-x'
    penalty = 1e8
    boundary = 'left right top bottom'
  []
[]

[AuxKernels]
  [current_density]
    type = ADCurrentDensity
    variable = J
    electrostatic = false
    electric_field = u
  []
[]

[Materials]                                 # THIS MATERIAL IS ONLY USED TO TEST THE CURRENT DENSITY CALCULATION
  [conductivity]                            # Electrical conductivity for graphite at 293.15 K in S/m
    type = ADGenericConstantMaterial        # perpendicular to basal plane
    prop_names = 'electrical_conductivity'  # Citation: H. Pierson, "Handbook of carbon, graphite,
    prop_values = 3.33e2                    #           diamond, and fullerenes: properties, processing,
  []                                        #           and applications," p. 61, William Andrew, 1993.
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
