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
  active = 'xyzt'
  [x]
    type = ReporterNearestPointAux
    variable = val
    coord_x = 'values1D/coordx'
    value = 'values1D/value'
    execute_on = 'initial timestep_end'
  []
  [xy]
    type = ReporterNearestPointAux
    variable = val
    coord_x = 'values2D/coordx'
    coord_y = 'values2D/coordy'
    value = 'values2D/value'
    execute_on = 'initial timestep_end'
  []
  [xyz]
    type = ReporterNearestPointAux
    variable = val
    coord_x = 'values3D/coordx'
    coord_y = 'values3D/coordy'
    coord_z = 'values3D/coordz'
    value = 'values3D/value'
    execute_on = 'initial timestep_end'
  []
  [xyzt]
    type = ReporterNearestPointAux
    variable = val
    coord_x = 'values4D/coordx'
    coord_y = 'values4D/coordy'
    coord_z = 'values4D/coordz'
    time = 'values4D/time'
    value = 'values4D/value'
    execute_on = 'initial timestep_end'
  []

  [errorv]
    type = ReporterNearestPointAux
    variable = val
    value = 'empty/val'
  []
  [errorx]
    type = ReporterNearestPointAux
    variable = val
    coord_x = 'values1D/coordx'
    value = 'values4D/value'
  []
  [errory]
    type = ReporterNearestPointAux
    variable = val
    coord_y = 'values1D/coordx'
    value = 'values4D/value'
  []
  [errorz]
    type = ReporterNearestPointAux
    variable = val
    coord_z = 'values1D/coordx'
    value = 'values4D/value'
  []
  [errort]
    type = ReporterNearestPointAux
    variable = val
    time = 'values1D/coordx'
    value = 'values4D/value'
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

[VectorPostprocessors]
  [empty]
    type = PointValueSampler
    variable = 'val'
    points = ''
    sort_by = 'x'
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
