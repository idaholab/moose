#
# This problem is taken from the Abaqus verification manual:
#   "1.5.8 Patch test for heat transfer elements"
#
# The temperature on the exterior nodes is -2e5+200x+100y.
#
# This gives a constant flux at all Gauss points.
#
# In addition, the temperature at all nodes follows the same formula.
#
# Node x         y          Temperature
#    1 1e3       0          0
#    2 1.00024e3 0          48
#    3 1.00018e3 3e-2       39
#    4 1.00004e3 2e-2       10
#    9 1.00008e3 8e-2       24
#   10 1e3       1.2e-1     12
#   14 1.00016e3 8e-2       40
#   17 1.00024e3 1.2e-1     60

[Problem]
  coord_type = RZ
[]

[Mesh]#Comment
  file = heat_conduction_patch_rz_quad8.e
[] # Mesh

[Functions]
  [./temps]
    type = ParsedFunction
    expression ='-2e5+200*x+100*y'
  [../]
[] # Functions

[Variables]

  [./temp]
    order = SECOND
    family = LAGRANGE
  [../]

[] # Variables

[Kernels]

  [./heat_r]
    type = HeatConduction
    variable = temp
  [../]

[] # Kernels

[BCs]

  [./temps]
    type = FunctionDirichletBC
    variable = temp
    boundary = 10
    function = temps
  [../]

[] # BCs

[Materials]

  [./heat]
    type = HeatConductionMaterial
    block = 1

    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
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

  [./Quadrature]
    order = THIRD
  [../]

[] # Executioner

[Outputs]
  exodus = true
[] # Outputs
