# Outputs a density relationship into an exodus file
# and into a CSV file.
# In the exodus file, the density will be a function of "x", and
# this "x" is actually porepressure
# In the CSV file you will find the density at the "x" point
# specified by you below.
#
# You may specify:
#  - the "type" of density in the UserObjects block
#  - the parameters of this density function in the UserObjects block
#  - the "x" point (which is porepressure) that you want to extract
#       the density at, if you want a value at a particular point
#  - the range of "x" values (which is porepressure values) may be
#       changed in the Mesh block, below


[UserObjects]
  [./density]
    type = RichardsDensityVDW
    a = 0.2303
    b = 4.31E-5
    molar_mass = 16.04246E-3
    temperature = 293
  [../]
[]

[Postprocessors]
  [./point_val]
    type = PointValue
    execute_on = timestep_begin
    # note this point must lie inside the mesh below
    point = '1 0 0'
    variable = density
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  # the following specify the range of porepressure
  xmin = -1E6
  xmax = 1E7
[]

############################
# You should not need to change any of the stuff below
############################


[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u_init]
    type = FunctionIC
    variable = u
    function = x
  [../]
[]

[AuxVariables]
  [./density]
  [../]
[]

[AuxKernels]
  [./density_AuxK]
    type = RichardsDensityAux
    variable = density
    density_UO = density
    execute_on = timestep_begin
    pressure_var = u
  [../]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  num_steps = 0
[]

[Outputs]
  file_base = density
  [./csv]
    type = CSV
  [../]
  [./exodus]
    type = Exodus
    hide = u
  [../]
[]
