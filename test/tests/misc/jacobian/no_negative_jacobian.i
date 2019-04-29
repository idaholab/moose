# The mesh is inverted using a prescribed displacement.
# However, due to use_displaced_mesh = false in the Kernel,
# libMesh does not throw a "negative jacobian" error
[Mesh]
  type = GeneratedMesh
  dim = 3
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
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
  dt = 0.5
  end_time = 1.5
[]
