# PiecewiseMultilinear function test in 3D with function depending on time
#
# This test uses a function on the unit cube.
# For t<=3 the function is unity at (x,y,z)=(0,0,0) and zero elsewhere
# For t>=7 the function is unity at (x,y,z)=(1,1,1) and zero elsewhere

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  nx = 2
  ymin = 0
  ymax = 1
  ny = 2
  zmin = 0
  zmax = 1
  nz = 2
[]

[Variables]
  [./dummy]
  [../]
[]

[Kernels]
  [./dummy_kernel]
    type = TimeDerivative
    variable = dummy
  [../]
[]

[AuxVariables]
  [./f]
  [../]
[]

[AuxKernels]
  [./f_AuxK]
    type = FunctionAux
    function = fourDa
    variable = f
  [../]
[]

[Functions]
  [./fourDa]
    type = PiecewiseMultilinear
    data_file = fourDa.txt
  [../]
[]


[Executioner]
  type = Transient
  dt = 1
  end_time = 10
[]

[Outputs]
  file_base = fourDa
  exodus = true
  hide = dummy
[]
