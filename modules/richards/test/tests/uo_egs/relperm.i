# Outputs a relative permeability curve into an exodus file
# and into a CSV file.
# In the exodus file, the relperm will be a function of "x", and
# this "x" is actually effective saturation.
# In the CSV file you will find the relperm at the "x" point
# specified by you below.
#
# You may specify:
#  - the "type" of relative permeability in the UserObjects block
#  - the parameters of this relative permeability curve in the UserObjects block
#  - the "x" point (which is effective saturation) that you want to extract
#       the relative permeability at, if you want a value at a particular point


[UserObjects]
  [./relperm]
    type = RichardsRelPermPower
    simm = 0.1
    n = 3
  [../]
[]

[Postprocessors]
  [./point_val]
    type = PointValue
    execute_on = timestep_begin
    point = '0.5 0 0'
    variable = relperm
  [../]
[]


############################
# You should not need to change any of the stuff below
############################

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = 0
  xmax = 1
[]

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
  [./relperm]
  [../]
[]

[AuxKernels]
  [./relperm_AuxK]
    type = RichardsRelPermAux
    variable = relperm
    relperm_UO = relperm
    execute_on = timestep_begin
    seff_var = u
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
  file_base = relperm
  [./csv]
    type = CSV
  [../]
  [./exodus]
    type = Exodus
    hide = u
  [../]
[]
