#set mesh
[Mesh]
  coord_type = 'RZ'
  rz_coord_axis = Y
    #file = 'initial.e'
  file = 'ns_initial.e'
[]


[Variables]
    [Dummy]
    []
[]
# PK
[AuxVariables]
  [flux]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_from_file_var = 'flux'
  []
[]



[Kernels]
 [Dummy]
    type = NullKernel
    variable = Dummy
 []
[]


[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = false
[]

[Postprocessors]
  [flux_int]
    type = ElementIntegralVariablePostprocessor
    execute_on = 'INITIAL TIMESTEP_END'
    variable = flux
  []
[]
