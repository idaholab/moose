#
# Three independent cubes are thermally loaded, one in x, one in y, and one in z.
# Each direction has a different thermal conductivity, resulting in a different
#   temperature at the side with the Neumann bc.
#
# For x: 100/1000 = 1e-1
# For y: 100/100  = 1e+0
# for z: 100/10   = 1e+1
#


[Mesh]#Comment
  file = heat_conduction_ortho.e
[] # Mesh

[Functions]
  [./1000]
    type = ParsedFunction
    value = 1000
  [../]
  [./100]
    type = ParsedFunction
    value = 100
  [../]
  [./10]
    type = ParsedFunction
    value = 10
  [../]
[]

[Variables]

  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]

[] # Variables

[Kernels]

  [./heat]
    type = AnisoHeatConduction
    variable = temp
  [../]

[] # Kernels

[BCs]

  [./temps]
    type = DirichletBC
    variable = temp
    boundary = 1
    value = 0
  [../]

  [./neum]
    type = NeumannBC
    variable = temp
    boundary = 2
    value = 100
  [../]

[] # BCs

[Materials]

  [./heat]
    type = AnisoHeatConductionMaterial
    block = 1

    specific_heat = 0.116
    thermal_conductivity_x_pp = tcx
    thermal_conductivity_y_pp = tcy
    thermal_conductivity_z_pp = tcz
  [../]

  [./density]
    type = Density
    block = 1
    density = 0.283
  [../]

[] # Materials

[Executioner]

  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'


  line_search = 'none'


  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10


  l_max_its = 20

[] # Executioner

[Outputs]
  output_initial = true
  exodus = true
  hide = 'tcx tcy tcz'
  print_linear_residuals = true
  print_perf_log = true
[] # Outputs

[Postprocessors]
  [./tcx]
    type = PlotFunction
    function = 1000
    outputs = none
  [../]
  [./tcy]
    type = PlotFunction
    function = 100
    outputs = none
  [../]
  [./tcz]
    type = PlotFunction
    function = 10
    outputs = none
  [../]
[]
