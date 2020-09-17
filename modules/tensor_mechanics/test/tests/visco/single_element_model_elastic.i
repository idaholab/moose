[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  elem_type = HEX8
  # displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  # [./creep_strain_xx]
  #   order = CONSTANT
  #   family = MONOMIAL
  # [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    # use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = stress
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  [./strain_xx]
    type = RankTwoAux
    variable = strain_xx
    rank_two_tensor = total_strain
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  # [./creep_strain_xx]
  #   type = RankTwoAux
  #   variable = creep_strain_xx
  #   rank_two_tensor = creep_strain
  #   index_j = 0
  #   index_i = 0
  #   execute_on = timestep_end
  # [../]
[]

[Functions]
  [./strain_history]
    type = PiecewiseLinear
    x = '0.00	0.01	0.02	0.03	0.05	0.09	0.17	0.33	0.65	0.97	1.61	2.89	4.69	6.48	8.28	10.07	13.60	17.12	20.64	24.17	27.69	34.74	41.79	55.88	69.98	84.07	98.16	112.26	126.35	147.00	167.64	188.29	208.93	229.57	262.94	296.31	329.67	363.04	396.41	429.77	463.14	512.22	561.31	610.39	659.47	708.56	757.64	855.81	953.97	1052.14	1150.31	1275.31	1400.31	1525.31	1650.31	1775.31	1900.31	2025.31	2150.31	2275.31	2400.31	2525.31	2650.31	2775.31	2900.31	3025.31	3150.31	3275.31	3400.31	3525.31	3650.31	3775.31	3900.31	4025.31	4150.31	4275.31	4400.31	4525.31	4650.31	4775.31	4900.31	5025.31	5150.31	5275.31	5400.31	5525.31	5650.31	5775.31	5900.31	6025.31	6150.31	6275.31	6400.31	6525.31	6650.31	6775.31	6900.31	7025.31	7150.31	7275.31	7400.31	7525.31	7650.31	7775.31	7900.31	8025.31	8150.31	8275.31	8400.31	8525.31	8650.31	8775.31	8900.31	9025.31	9150.31	9275.31	9400.31	9525.31	9650.31	9775.31	9900.31	10025.31	10150.31	10275.31	10400.31	10525.31	10650.31	10775.31	10900.31	11025.31	11150.31	11275.31	11400.31	11525.31	11650.31	11775.31	11900.31	12025.31	12150.31	12275.31	12400.31	12525.31	12650.31	12775.31	12900.31	13025.31	13150.31	13275.31	13400.31	13525.31	13650.31	13775.31	13900.31	14025.31	14150.31	14275.31	14400.31	14525.31	14650.31	14775.31	14900.31	15025.31	15150.31	15275.31	15400.31	15525.31	15650.31	15775.31	15900.31	16025.31	16150.31	16275.31	16400.31	16525.31	16650.31	16775.31	16900.31	17000.00'
    y = '-0.000196734	-0.000196724	-0.000196715	-0.000196706	-0.00019669	-0.000196661	-0.000196614	-0.000196549	-0.000196455	-0.000196373	-0.000196217	-0.00019593	-0.000195572	-0.00019526	-0.000194985	-0.000194741	-0.00019433	-0.000193986	-0.000193688	-0.000193421	-0.000193177	-0.000192735	-0.000192329	-0.000191581	-0.00019089	-0.000190245	-0.000189641	-0.000189075	-0.000188544	-0.000187827	-0.000187174	-0.00018658	-0.000186038	-0.000185544	-0.000184836	-0.000184222	-0.00018369	-0.000183226	-0.00018282	-0.000182464	-0.00018215	-0.000181752	-0.000181415	-0.000181126	-0.000180876	-0.000180655	-0.000180459	-0.000180124	-0.000179839	-0.000179587	-0.000179358	-0.000179091	-0.000178842	-0.000178607	-0.000178382	-0.000178167	-0.000177959	-0.000177759	-0.000177565	-0.000177378	-0.000177197	-0.000177021	-0.000176851	-0.000176686	-0.000176527	-0.000176372	-0.000176223	-0.000176078	-0.000175937	-0.000175801	-0.00017567	-0.000175542	-0.000175418	-0.000175298	-0.000175182	-0.000175069	-0.00017496	-0.000174855	-0.000174752	-0.000174653	-0.000174557	-0.000174464	-0.000174373	-0.000174286	-0.000174201	-0.000174119	-0.000174039	-0.000173962	-0.000173887	-0.000173814	-0.000173744	-0.000173676	-0.00017361	-0.000173546	-0.000173484	-0.000173423	-0.000173365	-0.000173308	-0.000173254	-0.000173201	-0.000173149	-0.000173099	-0.000173051	-0.000173004	-0.000172958	-0.000172914	-0.000172871	-0.00017283	-0.00017279	-0.000172751	-0.000172713	-0.000172676	-0.00017264	-0.000172606	-0.000172573	-0.00017254	-0.000172509	-0.000172478	-0.000172449	-0.00017242	-0.000172392	-0.000172365	-0.000172339	-0.000172314	-0.000172289	-0.000172265	-0.000172242	-0.000172219	-0.000172198	-0.000172176	-0.000172156	-0.000172136	-0.000172117	-0.000172098	-0.00017208	-0.000172062	-0.000172045	-0.000172029	-0.000172012	-0.000171997	-0.000171982	-0.000171967	-0.000171953	-0.000171939	-0.000171926	-0.000171912	-0.0001719	-0.000171888	-0.000171876	-0.000171864	-0.000171853	-0.000171842	-0.000171831	-0.000171821	-0.000171811	-0.000171802	-0.000171792	-0.000171783	-0.000171774	-0.000171766	-0.000171757	-0.000171749	-0.000171742	-0.000171734	-0.000171727	-0.000171719	-0.000171712	-0.000171706	-0.000171699	-0.000171693	-0.000171687	-0.000171681	-0.000171675	-0.000171669	-0.000171664	-0.000171658	-0.000171653	-0.000171649'
  [../]
[]

[BCs]
  [./symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./axial_load]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = strain_history
  [../]
[]

[Materials]
  # [./burgers]
  #   type = GeneralizedKelvinVoigtModel
  #   creep_modulus = '   1.52e06
  #                       5.32e12
  #                       6.15e04
  #                       6.86e04
  #                       4.48e04
  #                       1.05e12' # data from TAMU
  #   creep_viscosity = ' 1
  #                       10
  #                       100
  #                       1000
  #                       10000
  #                       100000'  # data from TAMU
  #   poisson_ratio = 0.2
  #   young_modulus = 33570.84903
  # [../]
  # [./stress]
  #   type = ComputeMultipleInelasticStress
  #   inelastic_models = 'creep'
  # [../]
  [./stress]
    type = ComputeLinearElasticStress
    elasticity_tensor = 33570.84903
    # inelastic_models = 'creep'
  [../]
  # [./creep]
  #   type = LinearViscoelasticStressUpdate
  # [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

# [UserObjects]
#   [./update]
#     type = LinearViscoelasticityManager
#     viscoelastic_model = burgers
#   [../]
# []

[Postprocessors]
  [./stress_xx]
    type = ElementAverageValue
    variable = stress_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./strain_xx]
    type = ElementAverageValue
    variable = strain_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  # [./creep_strain_xx]
  #   type = ElementAverageValue
  #   variable = creep_strain_xx
  #   block = 'ANY_BLOCK_ID 0'
  # [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  l_max_its  = 50
  l_tol      = 1e-10
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  dtmin = 0.01
  end_time = 17000
  [./TimeStepper]
    type = LogConstantDT
    first_dt = 0.1
    log_dt = 0.1
  [../]
[]

[Outputs]
  # file_base = single_element_creep_out
  # exodus = true

 perf_graph     = true
 csv = true
 [./Console]
   type = Console
 [../]
 [./Exo]
   type = Exodus
   elemental_as_nodal = true
 [../]
[]
