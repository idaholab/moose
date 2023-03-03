# Test implementation of passing constant thermal conductivity and specific heat values to SodiumProperties
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 1
    nx = 5
    ny = 5
  []
[]

[AuxVariables]
  [pressure]
    type = MooseVariableFVReal
    [./InitialCondition]
      type = FunctionIC
      function = k_function  #'1e5 + 1e5*x*y'
    []
  []
  [T]
    type = MooseVariableFVReal
    [./InitialCondition]
      type = FunctionIC
      function = '500 - 50*x*x*y'
    []
  []
[]

[Functions]
  [k_function]
    type = PiecewiseBilinear
    x = '300 400 600'
    y ='1e5 2e5 4e5'
    z ='12 14 18 12.2 14.2 18.3 12.4 14.5 18.6'
    axis = 0
  []
  [rho_function]
    type = PiecewiseBilinear
    x = '300 400 600'
    y ='1e5 2e5 4e5'
    z ='1850 1760 1550 1890 1725 1600 1950 1800 1700 '
    axis = 0
  []
  [mu_function]
    type = PiecewiseBilinear
    x = '300 400 600'
    y ='1e5 2e5 4e5'
    z ='2.4e-4 2e-4 1e-4 2.6e-4 2.3e-4 1.3e-4 3e-4 2.5e-4 1.6e-4'
    axis = 0
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = TemperaturePressureFunctionFluidProperties
      k = k_function
      rho = rho_function
      mu = mu_function
      cv = 1970
      cp = 1980
    []
  []
[]

[Materials]
  [fluid_props_to_mat_props]
    type = FluidPropertiesMaterialPT
    fp = fp
    outputs = exodus
    pressure = pressure
    temperature = T
  []
[]

[Executioner]
  type = Transient
  end_time = 0.1
  dt = 0.1
[]

[Problem]
  solve=false
[]

[Outputs]
  [exodus]
    type = Exodus
  []
[]
