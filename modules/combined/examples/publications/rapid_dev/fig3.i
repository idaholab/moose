#
# Fig. 3 input for 10.1016/j.commatsci.2017.02.017
# D. Schwen et al./Computational Materials Science 132 (2017) 36-45
# Comparison of an analytical (ca) and numerical (c) phase field interface
# profile. Supply the L parameter on the command line to gather the data for
# the inset plot.
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = ${L}
  xmin = -30
  xmax = 30
[]

[Functions]
  [./solution]
    type = ParsedFunction
    expression = 0.5*(1+tanh(x/2^0.5))
  [../]
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = solution
      #type = FunctionIC
      #function = if(x>0,1,0)
    [../]
  [../]
  [./w]
  [../]
[]

[AuxVariables]
  [./diff]
  [../]
  [./ca]
    [./InitialCondition]
      type = FunctionIC
      function = '0.5*(1+tanh(x/2^0.5))'
    [../]
  [../]
[]

[AuxKernels]
  [./diff]
    type = ParsedAux
    variable = diff
    expression = c-ca
    coupled_variables = 'c ca'
  [../]
[]

[Materials]
  [./F]
    type = DerivativeParsedMaterial
    property_name = F
    expression = 'c^2*(1-c)^2'
    coupled_variables = c
  [../]
[]

[Kernels]
  # Split Cahn-Hilliard kernels
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = 1
    w = w
  [../]
  [./wres]
    type = SplitCHWRes
    variable = w
    mob_name = 1
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
[]

[Postprocessors]
  [./L2]
    type = ElementL2Error
    function = solution
    variable = c
  [../]
[]

[VectorPostprocessors]
  [./c]
    type = LineValueSampler
    variable = 'c ca diff'
    start_point = '-10 0 0'
    end_point = '10 0 0'
    num_points = 200
    sort_by = x
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  end_time = 1e+6

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 5
    iteration_window = 1
  [../]
[]

[Outputs]
  csv = true
  execute_on = final
[]
