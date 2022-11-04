###########################################
#                                         #
#   THIS EXAMPLE CONTAINS AN EXCAVATION   #
#                                         #
###########################################


# Easiest way of figuring out what's happening:
# Run this example, load into paraview, take
# a slice through (0,0,0) with normal (0,0,1),
# colour by pressure and play the animation.


# This mesh has an interior sideset called excav_bdy
[Mesh]
  type = FileMesh
  file = ex01_input.e
[]



# This is a boundary condition acting on excav_bdy
# All it does is to set the pressure to p_excav=0
# at places on excav_bdy wherever excav_fcn tells it to.
[BCs]
  [./excav_bdy]
    type = RichardsExcav
    boundary = excav_bdy
    p_excav = 0.0
    variable = pressure
    excav_geom_function = excav_fcn
  [../]
[]



[Functions]
# excav_fcn controls where to set pressure=p_excav
# You supply start and end positions and times and
# by a linear interpolation these define the position
# of the coal face at all times
  [./excav_fcn]
    type = RichardsExcavGeom
    start_posn = '0 -500 0'
    start_time = 0
    end_posn = '0 -300 0'
    end_time = 6E6
    active_length = 1E4
  [../]

# mass_bal_fcn calculates the mass balance
  [./mass_bal_fcn]
    type = ParsedFunction
    expression = abs((mi-fout-mf)/2/(mi+mf))
    symbol_names = 'mi mf fout'
    symbol_values = 'mass_init mass_final flux_out'
  [../]

# initial pressure - unimportant in this example
  [./initial_pressure]
    type = ParsedFunction
    expression = -10000*(z-100)
  [../]
[]




# following is needed by postprocessors, kernels, etc
# unimportant in this example
[GlobalParams]
  richardsVarNames_UO = PPNames
[]




# following does the calculation of relevant
# masses and mass-flux to the excavation
[Postprocessors]

# note that this is calculated at beginning of timestep
  [./mass_init]
    type = RichardsMass
    variable = pressure
    execute_on = 'initial timestep_begin'
  [../]

# note this is calculated at end of timestep
  [./mass_final]
    type = RichardsMass
    variable = pressure
    execute_on = timestep_end
  [../]

# this is what calculates the mass flux to the excavation
# it is calculating it for boundary=excav_bdy, and the
# excavation time-dependence is set through the excav_fcn
  [./flux_out]
    type = RichardsExcavFlow
    boundary = excav_bdy
    variable = pressure
    excav_geom_function = excav_fcn
  [../]

# mass_bal just outputs the result to screen
  [./mass_bal]
    type = FunctionValuePostprocessor
    function = mass_bal_fcn
  [../]
[]




######################################
#                                    #
#  THE FOLLOWING STUFF IS STANDARD   #
#                                    #
######################################


[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E9
  [../]
  [./Seff1VG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1E-5
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0
    sum_s_res = 0
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1E+2
  [../]
[]


[Variables]
  active = 'pressure'
  [./pressure]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./p_ic]
    type = FunctionIC
    variable = pressure
    function = initial_pressure
  [../]
[]

[AuxVariables]
  [./Seff1VG_Aux]
  [../]
[]


[Kernels]
  active = 'richardsf richardst'
  [./richardst]
    type = RichardsMassChange
    variable = pressure
  [../]
  [./richardsf]
    type = RichardsFlux
    variable = pressure
  [../]
[]


[Materials]
  [./all]
    type = RichardsMaterial
    block = '1 2 3 4'
    viscosity = 1E-3
    mat_porosity = 0.1
    mat_permeability = '1E-15 0 0  0 1E-15 0  0 0 1E-15'
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    sat_UO = Saturation
    seff_UO = Seff1VG
    SUPG_UO = SUPGstandard
    gravity = '0 0 -10'
    linear_shape_fcns = true
  [../]
[]

[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = Seff1VG
    pressure_vars = pressure
  [../]
[]


[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-13 1E-14 10000 30'
  [../]
[]


[Executioner]
  type = Transient
  end_time = 6E6
  dt = 3E6
  solve_type = NEWTON
[]

[Outputs]
  file_base = ex01
  exodus = true
[]
