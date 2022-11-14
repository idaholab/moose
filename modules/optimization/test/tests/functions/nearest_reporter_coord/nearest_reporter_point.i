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
  [val]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [val_aux]
    type = FunctionAux
    variable = val
    function = 'xyzt'
    execute_on = 'initial timestep_end'
  []
[]

[Functions]
  active = 'xyzt'
  [xx]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values1D/coordx'
    value_name = 'values1D/value'
  []
  [xy]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values2D/coordx'
    y_coord_name = 'values2D/coordy'
    value_name = 'values2D/value'
  []
  [xyz]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values3D/coordx'
    y_coord_name = 'values3D/coordy'
    z_coord_name = 'values3D/coordz'
    value_name = 'values3D/value'
  []
  [xyzt]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values4D/coordx'
    y_coord_name = 'values4D/coordy'
    z_coord_name = 'values4D/coordz'
    time_name = 'values4D/time'
    value_name = 'values4D/value'
  []

  [errorv]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values4D/coordx'
    value_name = 'values1D/val'
  []
  [errorx]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values1D/coordx'
    y_coord_name = 'values4D/coordx'
    value_name = 'values4D/value'
  []
  [errory]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values4D/coordx'
    y_coord_name = 'values1D/coordx'
    value_name = 'values4D/value'
  []
  [errorz]
    type = NearestReporterCoordinatesFunction
    variable = val
    x_coord_name = 'values4D/coordx'
    z_coord_name = 'values1D/coordx'
    value_name = 'values4D/value'
  []
  [errort]
    type = NearestReporterCoordinatesFunction
    x_coord_name = 'values4D/coordx'
    time_name = 'values1D/coordx'
    value_name = 'values4D/value'
  []
[]

[Reporters]
  [values1D]
    type = ConstantReporter
    real_vector_names = 'coordx value'
    real_vector_values = '0.25 0.75;
                          0.00 1.00'
  []
  [values2D]
    type = ConstantReporter
    real_vector_names = 'coordx coordy value'
    real_vector_values = '0.25 0.75 0.25 0.75;
                          0.25 0.25 0.75 0.75;
                          0.00 1.00 2.00 3.00'
  []
  [values3D]
    type = ConstantReporter
    real_vector_names = 'coordx coordy coordz value'
    real_vector_values = '0.25 0.75 0.25 0.75 0.25 0.75 0.25 0.75;
                          0.25 0.25 0.75 0.75 0.25 0.25 0.75 0.75;
                          0.25 0.25 0.25 0.25 0.75 0.75 0.75 0.75;
                          0.00 1.00 2.00 3.00 4.00 5.00 6.00 7.00'
  []
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
  exodus = true
[]
