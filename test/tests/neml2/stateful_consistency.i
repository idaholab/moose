# Driver used to exercise NEML2Action's stateful-consistency guards. The model itself
# (models/stateful_input.i) takes the old value 'foo~1' as an input while never producing 'foo',
# which triggers guard 2. The [stateful_no_output] test runs this as-is; the [initialize_bad_output]
# test reuses it with cli_args to point initialize_outputs at a non-existent output (guard 1).
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
  input = 'models/stateful_input.i'
  # NEML2TestModel is hosted inside MOOSE as a Python module; importing it registers the type with
  # the embedded (cpp-eager) interpreter so the input file can reference it.
  load = 'models/test_models.py'
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
[]

[Materials]
  [B]
    type = GenericFunctionMaterial
    prop_names = 'B'
    prop_values = 'y+t'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
