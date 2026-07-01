# Verifies NEML2 derivative aliasing: an optional third element in 'derivatives' and
# 'parameter_derivatives' renames the resulting MOOSE material property. Uses
# LinearIsotropicHardening (isotropic_hardening = K * equivalent_plastic_strain), so
#   d(isotropic_hardening)/d(equivalent_plastic_strain) = K               -> aliased to 'dh_deps'
#   d(isotropic_hardening)/d(K)                         = equiv. pl. strain -> aliased to 'dh_dK'
# The AuxKernels read the *aliased* property names; had aliasing failed those properties would not
# exist (the defaults 'disotropic_hardening/d...' would) and the run would error.

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
    # (output, input, alias) and (output, parameter, alias): the third element renames the property.
    derivatives = 'isotropic_hardening equivalent_plastic_strain dh_deps'
    parameter_derivatives = 'isotropic_hardening K dh_dK'
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
  # The NEML2 input variable, provided by MOOSE as a per-element-varying field.
  [equivalent_plastic_strain]
    type = GenericFunctionMaterial
    prop_names = 'equivalent_plastic_strain'
    prop_values = '0.1*(x+y)+0.01'
  []
[]

[AuxVariables]
  [dh_deps]
    family = MONOMIAL
    order = CONSTANT
  []
  [dh_dK]
    family = MONOMIAL
    order = CONSTANT
  []
  [eps]
    family = MONOMIAL
    order = CONSTANT
  []
  [err_deps]
    family = MONOMIAL
    order = CONSTANT
  []
  [err_dK]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  # Read the *aliased* derivative property names.
  [dh_deps]
    type = MaterialRealAux
    variable = dh_deps
    property = 'dh_deps'
    execute_on = 'TIMESTEP_END'
  []
  [dh_dK]
    type = MaterialRealAux
    variable = dh_dK
    property = 'dh_dK'
    execute_on = 'TIMESTEP_END'
  []
  [eps]
    type = MaterialRealAux
    variable = eps
    property = 'equivalent_plastic_strain'
    execute_on = 'TIMESTEP_END'
  []
  # Must be ~0: d(isotropic_hardening)/d(equivalent_plastic_strain) == K == 1000.
  [err_deps]
    type = ParsedAux
    variable = err_deps
    coupled_variables = 'dh_deps'
    expression = 'abs(dh_deps - 1000)'
    execute_on = 'TIMESTEP_END'
  []
  # Must be ~0: d(isotropic_hardening)/d(K) == equivalent_plastic_strain.
  [err_dK]
    type = ParsedAux
    variable = err_dK
    coupled_variables = 'dh_dK eps'
    expression = 'abs(dh_dK - eps)'
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  [max_err_deps]
    type = ElementExtremeValue
    variable = err_deps
    execute_on = 'TIMESTEP_END'
  []
  [max_err_dK]
    type = ElementExtremeValue
    variable = err_dK
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
