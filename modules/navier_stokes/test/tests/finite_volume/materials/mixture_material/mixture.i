[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[AuxVariables]
  [fl]
    type = MooseVariableFVReal
  []
  [cp]
    type = MooseVariableFVReal
  []
  [k]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [FunctionIC]
    type = FunctionIC
    variable = fl
    function = 'x'
  []
[]

[AuxKernels]
  [cp_aux]
    type = FunctorAux
    functor = cp_mixture
    variable = cp
  []
  [k_aux]
    type = FunctorAux
    functor = k_mixture
    variable = k
  []
[]

[VectorPostprocessors]
  [cp]
    type = LineValueSampler
    start_point = '0.1 0 0'
    end_point = '0.9 0 0'
    num_points = 5
    variable = cp
    sort_by = x
  []
  [k]
    type = LineValueSampler
    start_point = '0.1 0 0'
    end_point = '0.9 0 0'
    num_points = 5
    variable = k
    sort_by = x
  []
  [fl]
    type = LineValueSampler
    start_point = '0.1 0 0'
    end_point = '0.9 0 0'
    num_points = 5
    variable = fl
    sort_by = x
  []
[]

[Functions]
  [cp_solid]
    type = ParsedFunction
    expression = '1 - x'
  []
  [cp_liquid]
    type = ParsedFunction
    expression = 'x'
  []
  [k_solid]
    type = ParsedFunction
    expression = '2 - 3*x'
  []
  [k_liquid]
    type = ParsedFunction
    expression = '3*x'
  []
[]

[FunctorMaterials]
  [eff_cp]
    type = NSFVMixtureFunctorMaterial
    phase_2_names = 'cp_solid k_solid'
    phase_1_names = 'cp_liquid k_liquid'
    prop_names = 'cp_mixture k_mixture'
    phase_1_fraction = fl
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
