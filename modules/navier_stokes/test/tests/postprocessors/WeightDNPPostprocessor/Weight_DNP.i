[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2.0
    ymin = 0
    ymax = 2.0
    nx = 19
    ny = 19
  []
[]

[Variables]
  [C]
      family = MONOMIAL
      order = CONSTANT
      fv = true
  []
[]


[AuxVariables]
  [flux]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  [C_time]
    type = FVTimeKernel
    variable = C
  []
[]

[Executioner]
  type = Transient
  dt = 0.001
  start_time = 0# ${fparse -dt}
  end_time = 0.0025
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-4
  l_max_its = 200
[]

[Outputs]
  exodus = true
  [outfile]
    type = CSV
    delimiter = ' '
  []
[]


[MultiApps]
  [Reader]
    type = FullSolveMultiApp
    input_files = "Reader.i"
    execute_on = 'initial timestep_end'
  []
[]


[Transfers]
   [pull_flux_inital]
    type = MultiAppShapeEvaluationTransfer
    from_multi_app = Reader 
    source_variable = power_scaled
    variable = flux
  [] 
  [pull_C_tot_inital]
    type = MultiAppShapeEvaluationTransfer
    from_multi_app = Reader 
    source_variable = C_tot
    variable = C
  [] 
[]

[Postprocessors]
  [power_int]
    #type = ElementExtremeValue
    type = ElementIntegralVariablePostprocessor
    #type = NodalSum
    execute_on = 'initial timestep_end'
    variable = flux
  []
  [C_int]
    #type = ElementExtremeValue
    type = ElementIntegralVariablePostprocessor
    #type = NodalSum
    variable = C
    execute_on = 'initial timestep_end'
 []
 [B]
     type = ElementL2Norm
     variable = flux
 []
 [A]
     type = WeightDNPPostprocessor
     variable = flux
     other_variable = C
     Norm = B
     lambda = 0.076221089
 []
[]
