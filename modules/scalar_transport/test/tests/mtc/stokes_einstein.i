# Ensure that all "check_" values evaluate to 1

p = 1e5 # Pa
T = 800 # K
M = 30e-3 # kg/mol
R = 8.31446 # J/mol/K
mu = 20e-6 # Pa*s
v = 1 # m/s
d = 10e-3 # m
radius = 1e-3

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0
  xmax = 1
[]

[FluidProperties]
  # Define fluid property with ideal gas so properties are known for calculating gold values
  [simple_fp]
    type = IdealGasFluidProperties
    molar_mass = ${replace M}
    mu = ${replace mu}
  []
[]

# Density will be a constant based on ideal gas law
rho = ${fparse p/R/T*M}

# Nonlinear variables include pressure, temperature, and velocity, used by the mass transfer coefficient
# model
[Variables]
  [pressure]
  []
  [velocity]
  []
  [temperature]
  []
[]

[ICs]
  [p_ic]
    type = FunctionIC
    variable = pressure
    function = ${replace p}
  []
  [v_ic]
    type = FunctionIC
    variable = velocity
    function = ${replace v}
  []
  [T_ic]
    type = FunctionIC
    variable = temperature
    function = ${replace T}
  []
[]

[Kernels]
  [pressure]
    type = NullKernel
    variable = pressure
  []
  [velocity]
    type = NullKernel
    variable = velocity
  []
  [temperature]
    type = NullKernel
    variable = temperature
  []
[]

[AuxVariables]
  [mtc]
  []
  [mtc_scalar]
    family = SCALAR
  []
[]

[AuxKernels]
  [mtc_aux]
    type = GasLiquidMassTransferAux
    variable = mtc
    p = pressure
    T = temperature
    d = ${replace d}
    fp = simple_fp
    fluid_velocity = velocity
    radius = ${replace radius}
    equation = StokesEinstein
  []
[]

[AuxScalarKernels]
  [mtc_aux_scalar]
    type = GasLiquidMassTransferScalarAux
    variable = mtc_scalar
    p = ${replace p}
    T = ${replace T}
    d = ${replace d}
    fp = simple_fp
    fluid_velocity = ${replace v}
    radius = ${replace radius}
    equation = StokesEinstein
  []
[]


[Executioner]
  type = Steady
[]

D = ${fparse 1.38064852e-23 * T / (6 * pi * mu * radius)}
Re = ${fparse rho*v*d/mu}
Sc = ${fparse mu/(rho*D)}

[Postprocessors]
  [mtc]
    type = PointValue
    variable = mtc
    point = '1 0 0'
    outputs = none
  []
  mtc_gold = ${fparse 0.023*pow(Re, 0.8) * pow(Sc, 0.4) * D/d}
  [mtc_gold]
    type = ConstantPostprocessor
    value = ${fparse mtc_gold}
    outputs = none
  []
  [mtc_scalar_post]
    type = ScalarVariable
    variable = mtc_scalar
    outputs = none
  []
  abs_tol = ${fparse mtc_gold*0.01}
  [check_helium_750]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = mtc
    value_b = mtc_gold
    absolute_tolerance = ${fparse abs_tol}
  []
  [check_mtc_scalar]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = mtc_scalar_post
    value_b = mtc_gold
    absolute_tolerance = ${fparse abs_tol}
  []
[]

[Outputs]
  exodus = false
  [csv]
    type = CSV
    start_time = 1
  []
[]
