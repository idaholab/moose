[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100

    xmax = 1
    ymax = 1
  []
[]
[Problem]
  solve = false
[]

[Materials]
  [multiple_sources]
    type = ReporterOffsetFunctionMaterial
    property_name = 'multiple_funcs'
    x_coord_name = 'constant/x'
    y_coord_name = 'constant/y'
    z_coord_name = 'constant/z'
    function = gauss
    outputs = exodus
    output_properties = multiple_funcs
  []
  [ad_multiple_sources]
    type = ADReporterOffsetFunctionMaterial
    property_name = 'ad_multiple_funcs'
    x_coord_name = 'constant/x'
    y_coord_name = 'constant/y'
    z_coord_name = 'constant/z'
    function = circle
    outputs = exodus
    output_properties = ad_multiple_funcs
  []

[]
[Reporters]
  [constant]
    type = ConstantReporter
    real_vector_names = 'x y z'
    real_vector_values = '0 0.25 0.5 0.45; 0 0.75 0.5 0.45; 0.0 0.0 0.0 0.0'
    execute_on = 'INITIAL'
  []
[]

[Functions]
  [gauss]
    type = ParsedFunction
    expression = 'exp(-2.0 *(x^2 + y^2 + z^2)/(beam_radii^2))'
    symbol_names = 'beam_radii'
    symbol_values = '0.1'
  []
  [circle]
    type = ParsedFunction
    expression = 'if(x^2 +y^2 +z^2< beam_radii^2,1,0)'
    symbol_names = 'beam_radii'
    symbol_values = '0.1'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = FINAL
[]
