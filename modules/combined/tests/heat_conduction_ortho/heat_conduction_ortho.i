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

[Variables]

  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]

[] # Variables

[Kernels]

  [./heat]
    type = HeatConduction
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
    type = HeatConductionMaterial
    block = 1

    specific_heat = 0.116
    thermal_conductivity_x = tcx
    thermal_conductivity_y = tcy
    thermal_conductivity_z = tcz
  [../]

  [./density]
    type = Density
    block = 1
    density = 0.283
  [../]

[] # Materials

[Executioner]

  type = Steady
  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -snes_type -snes_ls -ksp_gmres_restart'
  petsc_options_value = 'lu       ls         basic    101'

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

[Postprocessors]
  [./tcx]
    type = Reporter
    default = 1000
    output = none
  [../]
  [./tcy]
    type = Reporter
    default = 100
    output = none
  [../]
  [./tcz]
    type = Reporter
    default = 10
    output = none
  [../]
[]
