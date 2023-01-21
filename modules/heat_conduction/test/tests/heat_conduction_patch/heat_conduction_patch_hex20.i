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
# Node x          y          z          Temperature
#   1  1.000E+00  0.000E+00  1.000E+00  4.0000E+02
#   2  6.770E-01  3.050E-01  6.830E-01  3.0250E+02
#   3  3.200E-01  1.860E-01  6.430E-01  2.1120E+02
#   4  0.000E+00  0.000E+00  1.000E+00  2.0000E+02
#   5  1.000E+00  1.000E+00  1.000E+00  5.0000E+02
#   6  7.880E-01  6.930E-01  6.440E-01  3.5570E+02
#   7  1.650E-01  7.450E-01  7.020E-01  2.4790E+02
#   8  0.000E+00  1.000E+00  1.000E+00  3.0000E+02
#   9  8.385E-01  1.525E-01  8.415E-01  3.5125E+02
#  10  4.985E-01  2.455E-01  6.630E-01  2.5685E+02
#  11  1.600E-01  9.300E-02  8.215E-01  2.0560E+02
#  12  5.000E-01  0.000E+00  1.000E+00  3.0000E+02
#  13  1.000E+00  5.000E-01  1.000E+00  4.5000E+02
#  14  7.325E-01  4.990E-01  6.635E-01  3.2910E+02
#  15  2.425E-01  4.655E-01  6.725E-01  2.2955E+02
#  16  0.000E+00  5.000E-01  1.000E+00  2.5000E+02
#  17  8.940E-01  8.465E-01  8.220E-01  4.2785E+02
#  18  4.765E-01  7.190E-01  6.730E-01  3.0180E+02
#  19  8.250E-02  8.725E-01  8.510E-01  2.7395E+02
#  20  5.000E-01  1.000E+00  1.000E+00  4.0000E+02
#  21  1.000E+00  0.000E+00  0.000E+00  2.0000E+02
#  22  0.000E+00  0.000E+00  0.000E+00  0.0000E+00
#  23  8.260E-01  2.880E-01  2.880E-01  2.5160E+02
#  24  2.490E-01  3.420E-01  1.920E-01  1.2240E+02
#  25  1.000E+00  0.000E+00  5.000E-01  3.0000E+02
#  26  5.000E-01  0.000E+00  0.000E+00  1.0000E+02
#  27  0.000E+00  0.000E+00  5.000E-01  1.0000E+02
#  28  9.130E-01  1.440E-01  1.440E-01  2.2580E+02
#  29  1.245E-01  1.710E-01  9.600E-02  6.1200E+01
#  30  7.515E-01  2.965E-01  4.855E-01  2.7705E+02
#  31  5.375E-01  3.150E-01  2.400E-01  1.8700E+02
#  32  2.845E-01  2.640E-01  4.175E-01  1.6680E+02
#  33  2.730E-01  7.500E-01  2.300E-01  1.7560E+02
#  34  0.000E+00  1.000E+00  0.000E+00  1.0000E+02
#  35  2.610E-01  5.460E-01  2.110E-01  1.4900E+02
#  36  0.000E+00  5.000E-01  0.000E+00  5.0000E+01
#  37  2.190E-01  7.475E-01  4.660E-01  2.1175E+02
#  38  1.365E-01  8.750E-01  1.150E-01  1.3780E+02
#  39  0.000E+00  1.000E+00  5.000E-01  2.0000E+02
#  40  8.500E-01  6.490E-01  2.630E-01  2.8750E+02
#  41  8.380E-01  4.685E-01  2.755E-01  2.6955E+02
#  42  8.190E-01  6.710E-01  4.535E-01  3.2160E+02
#  43  5.615E-01  6.995E-01  2.465E-01  2.3155E+02
#  44  1.000E+00  1.000E+00  0.000E+00  3.0000E+02
#  45  1.000E+00  5.000E-01  0.000E+00  2.5000E+02
#  46  1.000E+00  1.000E+00  5.000E-01  4.0000E+02
#  47  9.250E-01  8.245E-01  1.315E-01  2.9375E+02
#  48  5.000E-01  1.000E+00  0.000E+00  2.0000E+02


[Mesh]#Comment
  file = heat_conduction_patch_hex20.e
[] # Mesh

[Functions]
  [./temps]
    type = ParsedFunction
    expression ='200*x+100*y+200*z'
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
[] # Output
