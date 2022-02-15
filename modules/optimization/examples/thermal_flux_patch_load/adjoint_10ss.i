[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = halfSphere.e
  []
[]

[Variables]
  [temperature]
  []
[]

[AuxVariables]
  [saved_t]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
    save_in = saved_t
  []
[]

#-----every adjoint problem should have these two
[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = temperature
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
  []
[]
[Reporters]
  [misfit]
    type=OptimizationData
  []
[]


[BCs]
  [round]
    type = ConvectiveFluxFunction
    boundary = round
    variable = temperature
    coefficient = 0.05
    T_infinity = 0.0
  []
  [flat]
    type = NeumannBC
    variable = temperature
    boundary = flat
    value = 0
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  nl_forced_its = 1
[]

[Postprocessors]
  [adjoint_0]
    type = SideIntegralVariablePostprocessor
    boundary = 100
    variable = temperature
  []
  [adjoint_1]
    type = SideIntegralVariablePostprocessor
    boundary = 101
    variable = temperature
  []
  [adjoint_2]
    type = SideIntegralVariablePostprocessor
    boundary = 102
    variable = temperature
  []
  [adjoint_3]
    type = SideIntegralVariablePostprocessor
    boundary = 103
    variable = temperature
  []
  [adjoint_4]
    type = SideIntegralVariablePostprocessor
    boundary = 104
    variable = temperature
  []
  [adjoint_5]
    type = SideIntegralVariablePostprocessor
    boundary = 105
    variable = temperature
  []
  [adjoint_6]
    type = SideIntegralVariablePostprocessor
    boundary = 106
    variable = temperature
  []
  [adjoint_7]
    type = SideIntegralVariablePostprocessor
    boundary = 107
    variable = temperature
  []
  [adjoint_8]
    type = SideIntegralVariablePostprocessor
    boundary = 108
    variable = temperature
  []
  [adjoint_9]
    type = SideIntegralVariablePostprocessor
    boundary = 109
    variable = temperature
  []
[]

[VectorPostprocessors]
  [adjoint_bc]
    type = VectorOfPostprocessors
    postprocessors = 'adjoint_0 adjoint_1
                      adjoint_2 adjoint_3
                      adjoint_4 adjoint_5
                      adjoint_5 adjoint_7
                      adjoint_6 adjoint_9'
  []
[]


[VectorPostprocessors]
  [data_pt]
    type = PointValueSampler
    variable = temperature
    points = '4 0 0
              2 2 2
              2 -2 2
              2 -2 -2
              2 2 -2'
    sort_by = id
  []
[]

[Outputs]
  console = true
  exodus = true
  file_base = 'adjoint'
[]
