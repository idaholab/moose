# Combined path: a MOOSE field is fed in as the NEML2 model parameter K (per quadrature point) AND
# its output-parameter-derivative d(isotropic_hardening)/d(K) is requested in the same evaluation.
# This is the inverse-problem / field-parametrization case (the executor calls both set_parameter and
# param_jacobian on a batched parameter). Since isotropic_hardening = K * equivalent_plastic_strain,
# d(isotropic_hardening)/d(K) = equivalent_plastic_strain regardless of K's value, so the parameter
# derivative must still equal the MOOSE equivalent_plastic_strain field everywhere.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[NEML2]
  eager = true
  input = 'model.i'
  [all]
    model = 'model'
    device = 'cpu'
    # Set K from a MOOSE field (per-qp) AND request its derivative in the same solve.
    parameters = 'K'
    parameter_types = 'MATERIAL'
    parameter_derivatives = 'isotropic_hardening K'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
[]

[Materials]
  [equivalent_plastic_strain]
    type = GenericFunctionMaterial
    prop_names = 'equivalent_plastic_strain'
    prop_values = '0.1*(x+y)+0.01'
  []
  [K]
    type = GenericFunctionMaterial
    prop_names = 'K'
    prop_values = '500*(1+x)'
  []
[]

[AuxVariables]
  [dh_dK]
    family = MONOMIAL
    order = CONSTANT
  []
  [eps]
    family = MONOMIAL
    order = CONSTANT
  []
  [abs_err]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [dh_dK]
    type = MaterialRealAux
    variable = dh_dK
    property = 'disotropic_hardening/dK'
    execute_on = 'TIMESTEP_END'
  []
  [eps]
    type = MaterialRealAux
    variable = eps
    property = 'equivalent_plastic_strain'
    execute_on = 'TIMESTEP_END'
  []
  [abs_err]
    type = ParsedAux
    variable = abs_err
    coupled_variables = 'dh_dK eps'
    expression = 'abs(dh_dK - eps)'
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  # Must be ~0: d(isotropic_hardening)/d(K) == equivalent_plastic_strain even with K set per-qp.
  [max_abs_err]
    type = ElementExtremeValue
    variable = abs_err
    execute_on = 'TIMESTEP_END'
  []
  [avg_dh_dK]
    type = ElementAverageValue
    variable = dh_dK
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 1
[]

[Outputs]
  csv = true
[]
