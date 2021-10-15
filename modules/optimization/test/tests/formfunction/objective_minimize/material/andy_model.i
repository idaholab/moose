# Heat conduction in a 1D bar of length 10
# Temperature is initialised to zero
# Temperature is fixed at temperature = 1 at the left-hand end, and no heat is allowed to escape from the right-hand end
# The analytical solution is known to be the error function, and has been well-tested in MOOSE.
# With diffusivity = 1, after 10 seconds of simulation, the temperature at the centre of the bar is 0.255
# (this agrees with the error-function analytical solution, up to spatio-temporal discretisation errors)
# 
# The purpose of this input file is to produce a prediction for the temperature at the centre of the bar.  It is run as a subapp of andy_main.i using various different values of the diffusivity.
#
# The unusual things about this input file are:
# - storing the diffusivity in a Postprocessor, which then gets fed to a ParsedFunction, which then gets fed to a GenericFunctionMaterial.  The reason for this is because the Postprocessor can be controlled by andy_main.i 
# - the existance of the diffusivityReceiver Control.  It is so the diffusivity can be passed to this file from andy_main.i
# - the existance of the MeasuredDataPointSampler VectorPostprocessor: it simply records the difference between the prediction for the temperature at the centre of the bar and the expected value of 0.255
#
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 10
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [dot]
    type = TimeDerivative
    variable = temperature
  []
  [conduction]
    type = MatDiffusion
    diffusivity = diffusivity
    variable = temperature
  [../]
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 1
  []
[]

[Functions]
  [diffusivity_function]
    type = ParsedFunction
    value = alpha
    vars = alpha
    vals = diffusivity_postprocessor
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = diffusivity
    prop_values = diffusivity_function
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1.0
  end_time = 10.0
[]

[Postprocessors]
  [diffusivity_postprocessor]
    type = ConstantValuePostprocessor
    value = 0.8
    execute_on = 'initial linear'
  []
  [T5]
    type = PointValue
    point = '5 0 0'
    variable = temperature
  []
[]

[VectorPostprocessors]
  [measuring_point]
    type = MeasuredDataPointSampler
    variable = temperature
    points = '5 0 0'
    measured_values = 0.25486206207717
  []
[]

[Controls]
  [diffusivityReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  console = false
  exodus = false
  csv=false
  file_base = andy_model
[]
