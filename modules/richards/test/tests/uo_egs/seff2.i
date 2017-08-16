# Outputs a 2phase effective saturation relationship into an exodus file
# and into a CSV file.
# In the exodus file, the Seff will be a function of "x", and
# this "x" is actually the difference in porepressures,
# say P_gas - P_water (so "x" should be positive).
# In the CSV file you will find the Seff at the "x" point
# specified by you below.
#
# You may specify:
#  - the "type" of Seff in the UserObjects block
#  - the parameters of this Seff function in the UserObjects block
#  - the "x" point (which is del_porepressure) that you want to extract
#       the Seff at, if you want a value at a particular point
#  - the range of "x" values (which is porepressure values) may be
#       changed in the Mesh block, below


[UserObjects]
  [./seff]
    type = RichardsSeff2waterVG
    al = 1E-6
    m = 0.8
  [../]
[]

[Postprocessors]
  [./point_val]
    type = PointValue
    execute_on = timestep_begin
    # note this point must lie inside the mesh below
    point = '1 0 0'
    variable = seff
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  # the following specify the range of porepressure
  xmin = 0
  xmax = 3E6
[]

############################
# You should not need to change any of the stuff below
############################


[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[ICs]
  [./u_init]
    type = FunctionIC
    variable = u
    function = x
  [../]
  [./v_init]
    type = ConstantIC
    variable = v
    value = 0
  [../]
[]

[AuxVariables]
  [./seff]
  [../]
[]

[AuxKernels]
  [./seff_AuxK]
    type = RichardsSeffAux
    variable = seff
    seff_UO = seff
    execute_on = timestep_begin
    pressure_vars = 'v u'
  [../]
[]

[Kernels]
  [./dummy_u]
    type = Diffusion
    variable = u
  [../]
  [./dummy_v]
    type = Diffusion
    variable = v
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  num_steps = 0
[]

[Outputs]
  file_base = seff2
  [./csv]
    type = CSV
  [../]
  [./exodus]
    type = Exodus
    hide = 'u v'
  [../]
[]
