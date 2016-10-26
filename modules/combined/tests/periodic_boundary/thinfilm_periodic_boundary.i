[Mesh]
  file = thinfilm.e
[]

[GlobalParams]
  use_displaced_mesh = false
  displacements = 'disp_x disp_y disp_z'
  prefactor = -0.01
[]


[MeshModifiers]
  [./cnode]
    type = AddExtraNodeset
    coord = '0.0 0.0 0.0' #these coords must be within some tolerance of a node (close!)
    new_boundary = 100
  [../]
  [./anode]
    type = AddExtraNodeset
    coord = '-1.0 -1.0 -1.0'
    new_boundary = 101
  [../]
[]


[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block  = '1'
    [./InitialCondition]
      type = RandomIC
      min = -0.5e-5
      max = 0.5e-5
    [../]
  [../]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]


[Materials]
  [./eigen_strain_zz] #Use for stress-free strain (ie epitaxial)
    type = ComputeEigenstrain
    block = '1'
    eigen_base = '1 0 0 0 1 0 0 0 0'
  [../]

  [./elasticity_tensor_1]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '380. 150. 150. 380. 150. 380. 110. 110. 110.'
    block = '1'
  [../]
  [./strain_1]
    type = ComputeSmallStrain
    block = '1'
  [../]
  [./stress_1]
    type = ComputeLinearElasticStress
    block = '1'
  [../]
  [./elasticity_tensor_2]
    type = ComputeElasticityTensor
    C_ijkl = '319 99.6 99.6 319 99.6 319 109.53 109.53 109.53'
    fill_method = symmetric9
    block = '2'
  [../]
  [./strain_2]
    type = ComputeSmallStrain
    block = '2'
  [../]
  [./stress_2]
    type = ComputeLinearElasticStress
    block = '2'
  [../]
[]



[Kernels]
  [./TensorMechanics]
    #This is an action block
  [../]
  [./diff]
    type = Diffusion
    variable = u
    block = '1'
  [../]
  [./u_time]
    type = TimeDerivative
    variable = u
    block = '1'
  [../]

[]


[BCs]

  # fix center point location
  [./centerfix_x]
    type = PresetBC
    boundary = 100
    variable = disp_x
    value = 0
  [../]
  [./centerfix_y]
    type = PresetBC
    boundary = 100
    variable = disp_y
    value = 0
  [../]
  [./centerfix_z]
    type = PresetBC
    boundary = 100
    variable = disp_z
    value = 0
  [../]

  # fix side point x coordinate to inhibit rotation
  [./angularfix_x]
    type = PresetBC
    boundary = 101
    variable = disp_x
    value = 0
  [../]
  [./angularfix_y]
    type = PresetBC
    boundary = 101
    variable = disp_y
    value = 0
  [../]

  [./u_bot]
    type = DirichletBC
    variable = u
    boundary = '7'
    value = 0.0
  [../]
  [./Periodic]
    [./TB_u_pbc]
      variable = u
      primary = '3'
      secondary = '5'
      translation = '0 2 0'
    [../]
    [./RL_u_pbc]
      variable = u
      primary = '4'
      secondary = '6'
      translation = '2 0 0'
    [../]
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options = '-snes_view -snes_linesearch_monitor -snes_converged_reason -ksp_converged_reason'
    petsc_options_iname = '-ksp_gmres_restart  -snes_rtol -ksp_rtol -pc_type'
    petsc_options_value = '    121                1e-6      1e-8    bjacobi'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'       #"PJFNK, JFNK, NEWTON"
  scheme = 'implicit-euler'   #"implicit-euler, explicit-euler, crank-nicolson, bdf2, rk-2"
  dtmin = 1e-13
  dtmax = 0.8
  num_steps = 3
[]

[Outputs]
  [./out]
    type = Exodus
    file_base = thinfilm_pb
    elemental_as_nodal = true
    interval = 1
  [../]
[]
