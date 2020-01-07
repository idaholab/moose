# The 2D mesh is inverted using a prescribed displacement.
# However, due to use_displaced_mesh = false in the Kernel,
# libMesh does not throw a "negative jacobian" error
[Mesh]
  type = GeneratedMesh
  dim = 2
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxKernels]
  [./disp_x]
    variable = disp_x
    type = FunctionAux
    function = '-x*t'
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
    use_displaced_mesh = false
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.8
  end_time = 1.5
[]
