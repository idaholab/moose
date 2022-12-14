[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
[]

[AuxVariables]
  [u]
    initial_condition = 1
  []
[]

[AuxKernels]
  [u_val]
    type = ParsedAux
    variable = u
    use_xyzt = true
    function = 't'
  []
[]

[Functions]
  [xyzt]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values4D/coordx'
    y_coord_name = 'values4D/coordy'
    z_coord_name = 'values4D/coordz'
    time_name = 'values4D/time'
    value_name = 'values4D/value'
  []
[]

[Reporters]
  [values4D]
    type = ConstantReporter
    real_vector_names = 'coordx coordy coordz time value'
    real_vector_values = '0.25 0.75 0.25 0.75 0.25 0.75 0.25 0.75
                          0.25 0.75 0.25 0.75 0.25 0.75 0.25 0.75
                          0.25 0.75 0.25 0.75 0.25 0.75 0.25 0.75;

                          0.25 0.25 0.75 0.75 0.25 0.25 0.75 0.75
                          0.25 0.25 0.75 0.75 0.25 0.25 0.75 0.75
                          0.25 0.25 0.75 0.75 0.25 0.25 0.75 0.75;

                          0.25 0.25 0.25 0.25 0.75 0.75 0.75 0.75
                          0.25 0.25 0.25 0.25 0.75 0.75 0.75 0.75
                          0.25 0.25 0.25 0.25 0.75 0.75 0.75 0.75;

                          0.00 0.00 0.00 0.00 0.00 0.00 0.00 0.00
                          0.50 0.50 0.50 0.50 0.50 0.50 0.50 0.50
                          1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00;

                          0.00 1.00 2.00 3.00 4.00 5.00 6.00 7.00
                          8.00 9.00 10.0 11.0 12.0 13.0 14.0 15.0
                          16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0'
    outputs = none
  []
[]

[VectorPostprocessors]
  [inner_product]
    type = ElementOptimizationSourceFunctionInnerProduct
    variable = u
    function = xyzt
    execute_on = 'initial timestep_end'
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
  skip_nl_system_check = true
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1
[]

[Outputs]
  csv = true
  execute_on = final
[]
