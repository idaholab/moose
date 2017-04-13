# Illustrates throwing an Exception from a Material.  In this case we
# don't actually recover from the segfault (so it is a RunException
# test) but in practice one could do so.  The purpose of this test is
# to ensure that exceptions can be thrown from Materials with stateful
# material properties without reading/writing to/from uninitialized
# memory.
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[BCs]
  [./bottomx]
    type = PresetBC
    variable = disp_x
    boundary = back
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    variable = disp_y
    boundary = back
    value = 0.0
  [../]
  [./bottomz]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]

  [./topx]
    type = FunctionPresetBC
    variable = disp_x
    boundary = front
    function = 0
  [../]
  [./topy]
    type = FunctionPresetBC
    variable = disp_y
    boundary = front
    function = 0
  [../]
  [./topz]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = t
  [../]
[]


[UserObjects]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 20
  [../]
  [./tanphi]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
  [./tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.1
  [../]
  [./t_strength]
    type = TensorMechanicsHardeningCubic
    value_0 = 1
    value_residual = 2
    internal_limit = 1
  [../]
  [./c_strength]
    type = TensorMechanicsHardeningConstant
    value = 100
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 1'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeCappedWeakPlaneStress
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    max_NR_iterations = 1
    tip_smoother = 5
    smoothing_tol = 5
    yield_function_tol = 1E-10
  [../]
[]

[Executioner]
  end_time = 1
  dt = 1
  dtmin = 1
  type = Transient
[]

[Outputs]
  file_base = SEGFAULT
  csv = true
[]

