# Generic driver exercising dump_inputs_on_failure: a plain diffusion problem
# whose reaction rate is a NEML2 model output. The NEML2 model (models/failing_model.i)
# is configured to always fail its constitutive update, so on the first nonlinear
# solve the executor serializes the batch of NEML2 input tensors to a per-rank
# TorchScript file. Uses only framework objects -- no solid_mechanics.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[NEML2]
  eager = true
  input = 'models/failing_model.i'
  dump_inputs_on_failure = true
  [all]
    model = 'model'
    device = 'cpu'
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
  [reaction]
    type = MatReaction
    variable = u
    reaction_rate = 'x' # NEML2 model output, drives one constitutive update per solve
  []
[]

[Materials]
  [forcing]
    type = GenericConstantMaterial
    prop_names = 'x_rate'
    prop_values = '1.0' # non-zero rate -> non-zero initial residual -> guaranteed failure
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1e-3
  dtmin = 1e-3 # do not sub-cut: the first failure aborts the (intentionally failing) run
  num_steps = 1
[]

[Outputs]
[]
