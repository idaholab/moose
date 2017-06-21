[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 200
  ny = 200
  elem_type = TRI3
[]

[Variables]
  [./phi]
    order  = FIRST
    family = LAGRANGE
  [../]

[]

[Kernels]
  active = 'diffusion reaction source'
  [./diffusion]
    type = Diffusion
    variable = phi
  [../]
  [./reaction]
    type = Reaction
    variable = phi
  [../]
  [./source]
    type = BodyForce
    variable = phi
  [../]
[]

[BCs]
[]

[Postprocessors]
  active = ''
  [./norm]
    type = ElementL2Norm
    variable = phi
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'
[]

[Outputs]
  exodus = true
  print_perf_log = true
[]
