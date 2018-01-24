# Test cases for convective boundary conditions.
# Input file for htc_3dtest1

# TKLarson
# 11/02/11
# Revision 0
#
# Goals of this test are:
#  1) show that the 'fluid' temperature for convective boundary condition
#    is behaving as expected/desired
#  2) show that expected results ensue from application of convective boundary conditions
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

# The mesh for this test case is concocted from an ASTM standard for the so-called Brazillian Cylinder test
# (ASTM International, Standard Test Method for Splitting Tensile Strength of Cylindrical Concrete
# Specimens, C 496/C 496M-04, 2004).  I turned a cylinder model into a rectangular parallelpiped,
# because I already had the cylinder model.
# The model is 3-d xyz coordinates.
#
# Brazillian Parallelpiped sample dimensions:
#       z = 10.3 cm, 0.103 m, (4 in)
#       y = 5.08 cm, 0.0508 m, (2 in)
#       x = 5.08 cm, 0.0508 m, (2 in)

# Material properties are:
#   density = 2405.28 km/m^3
#   specific heat = 826.4 J/kg-K
#   thermal conductivity 1.937 w/m-K
#  alpha (thermal conductivity/(density*specific heat) is then 9.74e-7 m^2/s
#
# Initial parallelpiped temperature is room temperature 294.26 K (70 F)
# The initial fluid temperature is room temperature. We will ramp it to 477.6 K (400 F) in 10 minutes.
# We will use an h representative of natural convection conditions as the boundary condition for all sides
# on the parallelpiped.  Akin to putting the object in an oven and turning the oven on.
#  This is essentially a thermal soak.
#
# What we expect for this problem:
#  1) Use of h = 284 w/m^2-K (50 BTU/hr-ft^2-F) should cause the parallelpiped to slowly heat up to 477K.
#  2) The fluid temperature should rise from initial (294.26 K) to final (477.6 K) in 600 s.
#  3) 1) and 2) should show the convective BC is working as desired.
#
[Mesh]    # Mesh Start
# 5cm x 5cm x 10cm parallelpiped not so detailed mesh, 4 elements each end, 8 elements each long face
# Only one block (Block 1), all concrete
# Sideset definitions:
#    1 - xy plane at z=0,
#    2 - xy plane at z=-0.103,
#    3 - xz plane at y=0,
#    4 - yz plane at x=0,
#    5 - xz plane at y=0.0508,
#    6 - yz plane at x=0.0508
  file = heat_convection_3d_mesh.e
#
[]    # Mesh END

[Variables]  # Variables Start
  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 294.26 # Initial parallelpiped temperature
  [../]

[]    # Variables END


[Kernels]  # Kernels Start
  [./heat]
#    type = HeatConductionRZ
     type = HeatConduction
     variable = temp
  [../]

  [./heat_ie]
#  type = HeatConductionTimeDerivativeRZ
  type = HeatConductionTimeDerivative
  variable = temp
  [../]

[]    # Kernels END


[BCs]    # Boundary Conditions Start
# Heat transfer coefficient on outer parallelpiped radius and ends
  [./convective_clad_surface]    # Convective Start
#         type = ConvectiveFluxRZ  # Convective flux, e.g. q'' = h*(Tw - Tf)
         type = ConvectiveFluxBC  # Convective flux, e.g. q'' = h*(Tw - Tf)
         boundary = '1 2 3 4 5 6'  # BC applied on top, along length, and bottom
         variable = temp
   rate = 284.      # convective heat transfer coefficient (w/m^2-K)[50 BTU/hr-ft^2-F]
         initial = 294.26    # initial ambient (lab or oven) temperature (K)
         final = 477.6      # final ambient (lab or oven) temperature (K)
   duration = 600.    # length of time in seconds that it takes the ambient
           #     temperature to ramp from initial to final
  [../]          # Convective End

[]    # BCs END

[Materials]    # Materials Start
  [./thermal]
    type = HeatConductionMaterial
    block = 1
    specific_heat = 826.4
    #thermal_conductivity = 1.937  # this makes alpha 9.74e-7 m^2/s
    thermal_conductivity = 193.7  # this makes alpha 9.74e-5 m^2/s
          # above conductivity arbitrarily increased by 2 decades to make the
          #   object soak faster for the present purposes

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
  file_base = out_3d
  exodus = true
[]      # Output END
#      # Input file END
