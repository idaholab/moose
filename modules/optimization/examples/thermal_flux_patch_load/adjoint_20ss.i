[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = halfSphere.e
  []
  [patch]
    type = PatchSidesetGenerator
    boundary = flat
    n_patches = 20
    input = fmg
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
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [adjoint_0]
    type = SideIntegralVariablePostprocessor
    boundary = flat_0
    variable = temperature
  []
  [adjoint_1]
    type = SideIntegralVariablePostprocessor
    boundary = flat_1
    variable = temperature
  []
  [adjoint_2]
    type = SideIntegralVariablePostprocessor
    boundary = flat_2
    variable = temperature
  []
  [adjoint_3]
    type = SideIntegralVariablePostprocessor
    boundary = flat_3
    variable = temperature
  []
  [adjoint_4]
    type = SideIntegralVariablePostprocessor
    boundary = flat_4
    variable = temperature
  []
  [adjoint_5]
    type = SideIntegralVariablePostprocessor
    boundary = flat_5
    variable = temperature
  []
  [adjoint_6]
    type = SideIntegralVariablePostprocessor
    boundary = flat_6
    variable = temperature
  []
  [adjoint_7]
    type = SideIntegralVariablePostprocessor
    boundary = flat_7
    variable = temperature
  []
  [adjoint_8]
    type = SideIntegralVariablePostprocessor
    boundary = flat_8
    variable = temperature
  []
  [adjoint_9]
    type = SideIntegralVariablePostprocessor
    boundary = flat_9
    variable = temperature
  []
  [adjoint_10]
    type = SideIntegralVariablePostprocessor
    boundary = flat_10
    variable = temperature
  []
  [adjoint_11]
    type = SideIntegralVariablePostprocessor
    boundary = flat_11
    variable = temperature
  []
  [adjoint_12]
    type = SideIntegralVariablePostprocessor
    boundary = flat_12
    variable = temperature
  []
  [adjoint_13]
    type = SideIntegralVariablePostprocessor
    boundary = flat_13
    variable = temperature
  []
  [adjoint_14]
    type = SideIntegralVariablePostprocessor
    boundary = flat_14
    variable = temperature
  []
  [adjoint_15]
    type = SideIntegralVariablePostprocessor
    boundary = flat_15
    variable = temperature
  []
  [adjoint_16]
    type = SideIntegralVariablePostprocessor
    boundary = flat_16
    variable = temperature
  []
  [adjoint_17]
    type = SideIntegralVariablePostprocessor
    boundary = flat_17
    variable = temperature
  []
  [adjoint_18]
    type = SideIntegralVariablePostprocessor
    boundary = flat_18
    variable = temperature
  []
  [adjoint_19]
    type = SideIntegralVariablePostprocessor
    boundary = flat_19
    variable = temperature
  []
[]

[VectorPostprocessors]
  [adjoint_bc]
    type = VectorOfPostprocessors
    postprocessors = 'adjoint_0 adjoint_1
                      adjoint_2 adjoint_3
                      adjoint_4 adjoint_5
                      adjoint_6 adjoint_7
                      adjoint_8 adjoint_9
                      adjoint_10 adjoint_11
                      adjoint_12 adjoint_13
                      adjoint_14 adjoint_15
                      adjoint_16 adjoint_17
                      adjoint_18 adjoint_19'
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
