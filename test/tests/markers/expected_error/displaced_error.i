[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  uniform_refine = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Steady
[]

[Adaptivity]
  [./Markers]
    [./test]
      type = UniformMarker
      # this triggers the expected error
      use_displaced_mesh = true
      mark = DONT_MARK
    [../]
  [../]
[]
