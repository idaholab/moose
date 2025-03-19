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
    load_during_construction = true
  []
[]

[Materials]
  [net_material]
    type = TorchScriptMaterial
    prop_names = diff_coeff
    input_names = 'pp1 pp2 pp3'
    torch_script_userobject = my_net
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
  [pp1]
    type = ConstantPostprocessor
    value = 0
  []
  [pp2]
    type = ConstantPostprocessor
    value = 0.1
  []
  [pp3]
    type = ConstantPostprocessor
    value = 0.2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [exo]
    type = Exodus
    output_material_properties = true
  []
[]
