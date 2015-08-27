# Test cases for convective boundary conditions. TKLarson, 11/01/11, rev. 0.
# Input file for htc_2dtest1

# TKLarson
# 11/01/11
# Revision 0
#
# Goals of this test are:
#  1) show that expected results ensue from application of convective boundary conditions
# Convective boundary condition:
#  q = h*A*(Tw - Tf)
#  where
#    q - heat transfer rate (w)
#    h - heat transfer coefficient (w/m^2-K)
#    A - surface area (m^2)
#    Tw - surface temperature (K)
#    Tf - fluid temperature adjacent to the surface (K)
# The heat transfer coefficient (h) is input as a variable called 'rate'
# Tf is a two valued function specified by 'initial' and 'final' along with a variable
#  called 'duration,' the length of time in seconds that it takes initial to linearly ramp
#  to 'final.'

# The mesh for this test case is based on an ASTM standard for the so-called Brazillian Cylinder test
# (ASTM International, Standard Test Method for Splitting Tensile Strength of Cylindrical Concrete
# Specimens, C 496/C 496M-04, 2004) (because I already had a version of the model).  While the
# Brazillian Cylinder test is for dynamic tensile testing of concrete, the model works for the present
# purposes.  The model is 2-d RZ coordinates.
#
# Brazillian Cylinder sample dimensions:
#       L = 20.3 cm, 0.203 m, (8 in)
#       r = 5.08 cm, 0.0508 m, (2 in)

# Material properties are:
#   density = 2405.28 km/m^3
#   specific heat = 826.4 J/kg-K
#   thermal conductivity 1.937 w/m-K
#  alpha (thermal conductivity/(density*specific heat) is then 9.74e-7 m^2/s
#
# Initial cylinder temperature is room temperature 294.26 K (70 F)
# The initial fluid temperature is room temperature. We will ramp it to 477.6 K (400 F) in 10 minutes.
# We will use a natural convection h (284 w/m^2-K (50 BTU/hr-ft^2-F)) on all faces of the cylinder.
# This is akin to putting the cylinder in an oven (nonconvection type) and turning the oven on.

# What we expect for this problem:
#  1) Use of h = 284 should cause the cylinder to slowly warm up
#  2) The fluid temperature should rise from initial (294 K) to final (477 K) in 600 s.
#  3) 1) and 2) should cause the cylinder to become soaked at 477.6 K after sufficient time(i.e. ~ 1/2 hr).
# This is a simple thermal soak problem.

[Problem]
  coord_type = RZ
[]

[Mesh]    # Mesh Start
# 10cm x 20cm cylinder not so detailed mesh, 2 radial, 6 axial nodes
# Only one block (Block 1), all concrete
# Sideset 1 - top of cylinder, Sideset 2 - length of cylinder, Sideset 3 - bottom of cylinder
  file = heat_convection_rz_mesh.e
[]    # Mesh END

[Variables]  # Variables Start
  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 294.26 # Initial cylinder temperature
  [../]

[]    # Variables END


[Kernels]  # Kernels Start
  [./heat]
    type = HeatConduction
    variable = temp
  [../]

  [./heat_ie]
    type = HeatConductionTimeDerivative
    variable = temp
  [../]

[]    # Kernels END


[BCs]    # Boundary Conditions Start
# Heat transfer coefficient on outer cylinder radius and ends
  [./convective_clad_surface]    # Convective Start
         type = ConvectiveFluxBC  # Convective flux, e.g. q'' = h*(Tw - Tf)
         boundary = '1 2 3'    # BC applied on top, along length, and bottom
         variable = temp
   rate = 284.      # (w/m^2-K)[50 BTU/hr/-ft^2-F]
          # the above h is a reasonable natural convection value
         initial = 294.26    # initial ambient (lab or oven) temperature (K)
         final = 477.6      # final ambient (lab or oven) temperature (K)
   duration = 600.    # length of time in seconds that it takes the ambient
          #   temperature to ramp from initial to final
  [../]          # Convective End

[]    # BCs END

[Materials]    # Materials Start
  [./thermal]
    type = HeatConductionMaterial
    block = 1
    specific_heat = 826.4
#    thermal_conductivity = 1.937  # this makes alpha 9.74e-7 m^2/s
#    thermal_conductivity = 19.37  # this makes alpha 9.74e-6 m^2/s
          # thermal conductivity arbitrarily increased by a decade to
          #    make the cylinder thermally soak faster (only for the purposes
          #    of this test problem
    thermal_conductivity = 193.7  # this makes alpha 9.74e-5 m^2/s
          # thermal conductivity arbitrarily increased by 2 decade to
          #    make the cylinder thermally soak faster (only for the purposes
          #    of this test problem

  [../]
  [./density]
    type = Density
    block = 1
    density = 2405.28
  [../]

[]      # Materials END

[Executioner]    # Executioner Start
   type = Transient
#   type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


   petsc_options = '-snes_ksp_ew '
   petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type'
   petsc_options_value = '70 hypre boomeramg'
   l_max_its = 60
   nl_rel_tol = 1e-8
   nl_abs_tol = 1e-10
   l_tol = 1e-5

   start_time = 0.0
  dt = 60.
  num_steps = 20  # Total run time 1200 s

[]      # Executioner END

[Outputs]    # Output Start
  # Output Start
  file_base = out_rz
  exodus = true
[]      # Output END
#      # Input file END
