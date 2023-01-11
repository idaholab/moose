[Mesh]    # Mesh Start
  file = patch_3d.e
#
[]    # Mesh END

[Functions]
  [./t_infinity]
    type = ParsedFunction
    expression = '300'
  [../]
  [./htc]
    type = ParsedFunction
    expression = 10.0*5.7                 # convective heat transfer coefficient (w/m^2-K)[50 BTU/hr-ft^2-F]
  [../]
[]

[Variables]  # Variables Start
  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 294.26
  [../]
[]    # Variables END


[Kernels]  # Kernels Start
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]    # Kernels END


[BCs]    # Boundary Conditions Start
# Heat transfer coefficient on outer parallelpiped radius and ends
  [./convective_clad_surface]    # Convective Start
    type = ConvectiveFluxFunction  # Convective flux, e.g. q'' = h*(Tw - Tf)
    boundary = 12
    variable = temp
    coefficient = htc
    T_infinity = t_infinity
  [../]                                  # Convective End

  [./fixed]
    type = DirichletBC
    variable = temp
    boundary = 10
    value = 100
  [../]
[]    # BCs END

[Materials]    # Materials Start
  [./thermal]
    type = HeatConductionMaterial
    block = '1 2 3 4 5 6 7'
    specific_heat = 826.4
    thermal_conductivity = 57
  [../]

  [./density]
    type = Density
    block = '1 2 3 4 5 6 7'
    density = 2405.28
  [../]
[]      # Materials END

[Executioner]    # Executioner Start
   type = Transient

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
   dt = 1
   num_steps = 1
[]      # Executioner END

[Outputs]    # Output Start
  # Output Start
  exodus = true
[]      # Output END
#      # Input file END
