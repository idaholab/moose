[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[UserObjects]
  [my_net]
    type = TorchScriptUserObject
    filename = "my_net.pt"
    execute_on = "TIMESTEP_BEGIN"
  []
[]

[Materials]
  [net_material]
    type = TorchScriptMaterial
    prop_names = diff_coeff
    input_names = 'my_pp1 my_pp2 my_pp3'
    torch_script_uo = my_net
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    diffusivity = diff_coeff
    variable = u
  []
  [source]
    type = BodyForce
    value = 50
    variable = u
  []
[]

[BCs]
  [dir_left]
    type = DirichletBC
    value = 10
    boundary = left
    variable = u
  []
[]

[Postprocessors]
  [my_pp1]
    type = ConstantPostprocessor
    value = 0
  []
  [my_pp2]
    type = ConstantPostprocessor
    value = 0.1
  []
  [my_pp3]
    type = ConstantPostprocessor
    value = 0.2
  []
  [average]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
