[Mesh]#Comment
  file = nodal_area_3D.e
[] # Mesh

[Variables]

  [./dummy]
    order = FIRST
    family = LAGRANGE
  [../]

[] # Variables

[AuxVariables]
  [./nodal_area]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]

  [./dummy]
    type = Diffusion
    variable = dummy
  [../]

[] # Kernels

[AuxKernels]
  [./nodal_area]
    type = NodalAreaAux
    variable = nodal_area
    nodal_area_object = nodal_area
  [../]
[]

[UserObjects]
  [./nodal_area]
    type = NodalArea
    variable = dummy
    boundary = 1
  [../]
[]

[BCs]

  [./dummy]
    type = DirichletBC
    variable = dummy
    boundary = 1
    value = 100
  [../]

[] # BCs

[Executioner]

  type = Steady
  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -snes_type -snes_ls -snes_linesearch_type -ksp_gmres_restart'
  petsc_options_value = 'jacobi   ls         basic    basic                    101'

  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10


  l_max_its = 20

[] # Executioner

[Output]
  interval = 1
  output_initial = true
  exodus = true
  perf_log = true
[] # Output
