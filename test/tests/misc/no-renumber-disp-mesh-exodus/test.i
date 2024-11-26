[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = 'mesh.inp'
    allow_renumbering = false
  []
[]

[Variables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
[]

[Kernels]
  [disp_x]
    type = Diffusion
    variable = disp_x
    use_displaced_mesh = true # Need a kernel to trigger a displaced element reinit
  []
  [disp_y]
    type = Diffusion
    variable = disp_y
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  start_time = 0.0
  num_steps = 1
  dt = 1
  dtmin = 1
[]

[Outputs]
  [ref]
    type = Exodus
  []
  [disp]
    type = Exodus
    use_displaced = true
  []
[]
