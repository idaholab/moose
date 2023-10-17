#
# This problem is taken from the Abaqus verification manual:
#   "1.5.8 Patch test for heat transfer elements"
#
# The temperature on the exterior nodes is 200x+100y+200z.
#
# This gives a constant flux at all Gauss points.
#
# In addition, the temperature at all nodes follows the same formula.
#
# Node x         y         z        Temperature
#   1  1.00E+00  0.00E+00  1.00E+00  400
#   2  6.77E-01  3.05E-01  6.83E-01  302.5
#   3  3.20E-01  1.86E-01  6.43E-01  211.2
#   4  0.00E+00  0.00E+00  1.00E+00  200
#   5  1.00E+00  1.00E+00  1.00E+00  500
#   6  7.88E-01  6.93E-01  6.44E-01  355.7
#   7  1.65E-01  7.45E-01  7.02E-01  247.9
#   8  0.00E+00  1.00E+00  1.00E+00  300
#   9  1.00E+00  0.00E+00  0.00E+00  200
#  10  0.00E+00  0.00E+00  0.00E+00  0
#  11  8.26E-01  2.88E-01  2.88E-01  251.6
#  12  2.49E-01  3.42E-01  1.92E-01  122.4
#  13  2.73E-01  7.50E-01  2.30E-01  175.6
#  14  0.00E+00  1.00E+00  0.00E+00  100
#  15  8.50E-01  6.49E-01  2.63E-01  287.5
#  16  1.00E+00  1.00E+00  0.00E+00  300

[Mesh]#Comment
  file = heat_conduction_patch.e
[] # Mesh

[Functions]
  [./temps]
    type = ParsedFunction
    expression ='200*x+100*y+200*z'
  [../]
[] # Functions

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

[] # Executioner

[Outputs]
  exodus = true
[] # Output
