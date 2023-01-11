# This input file generates an Exodus output file with a surrogate power profile
# that is used in the RELAP-7 run.  There is dummy diffusion solve to step through
# the simulation. The power profile (given as power density) is generated via
# aux variable

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0.020652
  xmax = 0.024748
  ymin = 0
  ymax = 3.865
  nx = 5
  ny = 20
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Functions]
  [power_density_fn]
    type = ParsedFunction
    expression = 'sin(y/3.865*pi)*sin((x-0.020652)/4.096e-3*pi/2.)*10e7*t'
  []
[]

[AuxVariables]
  [power_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [pd_aux]
    type = FunctionAux
    variable = power_density
    function = power_density_fn
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 0.1
  dt = 0.01
  abort_on_solve_fail = true
[]

[Outputs]
  [expdus]
    type = Exodus
    file_base = power_profile
  []
[]
