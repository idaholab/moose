# Verifies the NEML2 model-parameter-setting path: NEML2Action feeds a MOOSE field as the NEML2
# model parameter K (per quadrature point). Since isotropic_hardening = K * equivalent_plastic_strain,
# the NEML2 output must equal (MOOSE K) * (MOOSE equivalent_plastic_strain) everywhere. This also
# exercises the executor's per-step set_parameter + jacobian() with a batched (per-qp) parameter.

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
    verbose = true
    device = 'cpu'
    # Feed the MOOSE material 'K' in as the NEML2 model parameter ('K' resolves to 'model.K').
    parameters = 'K'
    parameter_types = 'MATERIAL'
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
  [h]
    family = MONOMIAL
    order = CONSTANT
  []
  [expected]
    family = MONOMIAL
    order = CONSTANT
  []
  [abs_err]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [h]
    type = MaterialRealAux
    variable = h
    property = 'isotropic_hardening'
    execute_on = 'TIMESTEP_END'
  []
  [expected]
    type = ParsedAux
    variable = expected
    expression = '(500*(1+x)) * (0.1*(x+y)+0.01)'
    use_xyzt = true
    execute_on = 'TIMESTEP_END'
  []
  [abs_err]
    type = ParsedAux
    variable = abs_err
    coupled_variables = 'h expected'
    expression = 'abs(h - expected)'
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  # Must be ~0: isotropic_hardening == K(MOOSE) * equivalent_plastic_strain(MOOSE).
  [max_abs_err]
    type = ElementExtremeValue
    variable = abs_err
    execute_on = 'TIMESTEP_END'
  []
  [avg_h]
    type = ElementAverageValue
    variable = h
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
