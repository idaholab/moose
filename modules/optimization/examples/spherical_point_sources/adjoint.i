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
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
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
