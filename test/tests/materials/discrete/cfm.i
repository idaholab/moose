# Illustrates the use of the ComputeFalseMaterial
[Materials]
  [./compute_false]
    type = ComputeFalseMaterial
  [../]
  [./use_compute_false]
    type = UseComputeFalseMaterial
    compute_false_material = 'compute_false'
  [../]
[]

[DiracKernels]
  [./dirac]
    type = MaterialPointSource
    point = '0.5 0 0'
    material_prop = compute_false_scalar
    variable = u
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]
