[Mesh]
 [fmg]
   type = FileMeshGenerator
   file = halfSphere.e
 []
 [patch]
  type = PatchSidesetGenerator
  boundary = flat
  n_patches = 10
  input = fmg
[]
[]

[Variables]
  [temperature]
  []
[]

[AuxVariables]
  [saved_t]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
    save_in = saved_t
  []
[]

[BCs]
  [round]
    type = ConvectiveFluxFunction
    boundary = round
    variable = temperature
    coefficient = 0.05
    T_infinity = 100.0
  []
  [surrogate_flat]
    type = FunctionNeumannBC
    variable = temperature
    boundary = flat
    function = y*4+20
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Problem]#do we need this
  type = FEProblem
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [measure_pt]
    type = PointValueSampler
    variable = temperature
    points = '4.24	0	2.45
     4.24	2.45	0
     4.24	0	-2.45
     4.24	-2.45	0
     2.45	0	4.24
     2.45	4.24	0
     2.45	0	-4.24
     2.45	-4.24	0
     4.9 0 0'
    sort_by = id
  []
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]

[Outputs]
  console = true #false
  exodus = true
  csv = true
  file_base = 'surrogate'
[]
