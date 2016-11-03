#
# Test the the getDefaultMaterialProperty in DerivativeMaterialInterface.
# This test should only pass, if the construction order of the Materials
# using this interface does not influence the outcome.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 0.1
  elem_type = QUAD4
[]

[GlobalParams]
  derivative_order = 2
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0.0
      y1 = 0.0
      radius = 0.5
      invalue = 1.0
      outvalue = 0.0
      int_width = 1.0
    [../]
  [../]
[]

[Kernels]
  [./dummy1]
    type = Diffusion
    variable = c
  [../]
  [./dummy2]
    type = TimeDerivative
    variable = c
  [../]
[]

[Materials]
  # derivatives used both before and after being declared
  [./sum_a_1]
    type = DerivativeSumMaterial
    f_name = Fa1
    sum_materials = 'Fa'
    args = 'c'
    outputs = exodus
  [../]
  [./free_energy_a]
    type = DerivativeParsedMaterial
    f_name = Fa
    args = 'c'
    function = 'c^4'
  [../]
  [./sum_a_2]
    type = DerivativeSumMaterial
    f_name = Fa2
    sum_materials = 'Fa'
    args = 'c'
    outputs = exodus
  [../]

  # derivatives declared after being used
  [./sum_b_1]
    type = DerivativeSumMaterial
    f_name = Fb1
    sum_materials = 'Fb'
    args = 'c'
    outputs = exodus
  [../]
  [./free_energy_b]
    type = DerivativeParsedMaterial
    f_name = Fb
    args = 'c'
    function = 'c^4'
  [../]

  # derivatives declared before being used
  [./free_energy_c]
    type = DerivativeParsedMaterial
    f_name = Fc
    args = 'c'
    function = 'c^4'
  [../]
  [./sum_c_2]
    type = DerivativeSumMaterial
    f_name = Fc2
    sum_materials = 'Fc'
    args = 'c'
    outputs = exodus
  [../]

  # non-existing derivatives
  [./free_energy_d]
    type = ParsedMaterial
    f_name = Fd
    args = 'c'
    function = 'c^4'
  [../]
  [./sum_d_1]
    type = DerivativeSumMaterial
    f_name = Fd1
    sum_materials = 'Fd'
    args = 'c'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'NEWTON'
  num_steps = 1
  dt = 1e-5
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
