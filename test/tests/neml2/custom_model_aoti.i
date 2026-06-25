# cpp-aoti counterpart of custom_model.i: the same NEML2TestModel is evaluated through the
# ahead-of-time-compiled artifact instead of the embedded (cpp-eager) interpreter. The artifact
# is produced by the 'aoti_compile' test (neml2-compile), so this input has no source '.i', no
# Python 'load', and no 'eager' flag (cpp-aoti is the default).
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[NEML2]
  # cpp-aoti runtime (default): load the compiled-artifact stub produced by neml2-compile (the
  # 'aoti_compile' test). Both the setup-time introspection and the runtime evaluation read it via
  # neml2::aoti::load_model -- no source .i, no Python at runtime.
  input = 'aoti_artifacts/model_aoti.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

    # request derivatives (must be pairs of two)
    # derivative name follow moose convention, e.g., 'doutput/dinput'
    derivatives = 'product A'

    # output to exodus
    export_outputs = 'sum product dproduct/dA'
    export_output_targets = 'exodus; exodus; exodus'
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
  [reaction_1]
    type = MatReaction
    variable = u
    reaction_rate = sum
  []
  [reaction_2]
    type = MatReaction
    variable = u
    reaction_rate = product
  []
  [reaction_3]
    type = MatReaction
    variable = u
    reaction_rate = dproduct/dA
  []
[]

[AuxVariables]
  [A]
  []
[]

[ICs]
  [A]
    type = FunctionIC
    variable = A
    function = 'x'
  []
[]

[Materials]
  [B]
    type = GenericFunctionMaterial
    prop_names = 'B'
    prop_values = 'y+t'
    outputs = 'exodus'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
