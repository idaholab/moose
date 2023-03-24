[Mesh]
  [mesh]
    type = CoarseMeshExtraElementIDGenerator
    input = abtr_mesh
    coarse_mesh = abtr_mesh
    extra_element_id_name = coarse_element_id
  []
[]

[TransportSystems]
  particle = neutron
  G = 33
  VacuumBoundary = 'top bottom radial'
  equation_type = eigenvalue
  [sn]
    scheme = DFEM-SN
    family = L2_LAGRANGE
    order = FIRST
    AQtype = Gauss-Chebyshev
    NPolar = 3
    NAzmthl = 4
    NA = 2
    sweep_type = asynchronous_parallel_sweeper
    using_array_variable = true
    collapse_scattering  = true
  []
[]

[Executioner]
  type = SweepUpdate
  verbose = true

  richardson_value = fission_source_integral
  richardson_abs_tol = 1e-6
  richardson_max_its = 500
  inner_solve_type = GMRes
  max_inner_its = 8
  debug_richardson = true

  cmfd_acceleration = false
  coarse_element_id = coarse_element_id
  diffusion_eigen_solver_type = krylovshur
  max_diffusion_coefficient = 10
#  cmfd_max_procs = 167
[]

[GlobalParams]
  library_file = mcc3.abtr.33macro3d_upd.xml
  library_name =  ISOTXS-neutron
  densities = 1.0
  plus = true
  dbgmat = false
  grid_names = 'Tfuel'
  grid = '1'
[]

[UserObjects]
  active = ''
  [flux]
    type = RattlesnakeVTKWriter
    var = 'flux_moment_g0_L0_M0 flux_moment_g1_L0_M0 flux_moment_g2_L0_M0 flux_moment_g3_L0_M0 flux_moment_g4_L0_M0 flux_moment_g5_L0_M0
           flux_moment_g6_L0_M0 flux_moment_g7_L0_M0 flux_moment_g8_L0_M0 flux_moment_g9_L0_M0 flux_moment_g10_L0_M0'
    cut = 2
    execute_on = final
  []
[]

[Materials]
  [icore]
    type = MixedNeutronicsMaterial
    material_id = 1
    block = '  84   85   80   81   79   75   76   82  104  105  100  101   99   95   96  102  124  125  120  121  119  115  116  122  144   145  140  141  139  135  136  142  167  168  162  164  161  156  157  165'
    isotopes = 'pseudo_ICORE'
  []
  [mcore]
    type = MixedNeutronicsMaterial
    material_id = 2
    block = '  83   74  103   94  123  114  143  134  166  155'
    isotopes = 'pseudo_MCORE'
  []
  [ocore]
    type = MixedNeutronicsMaterial
    material_id = 3
    block = '  73   68   69   70   71   77   72   66   67   78   93   88   89   90   91   97   92   86   87   98  113  108  109  110  111   117  112  106  107  118  133  128  129  130  131  137  132  126  127  138  154  148  149  151  152  158  153  146  147  159'
    isotopes = 'pseudo_OCORE'
  []
  [i1]
    type = MixedNeutronicsMaterial
    material_id = 4
    block = '  65   64   63   62'
    isotopes = 'pseudo_I1'
  []
  [i2]
    type = MixedNeutronicsMaterial
    material_id = 5
    block = ' 169  163  160  150'
    isotopes = 'pseudo_I2'
  []
  [i3]
    type = MixedNeutronicsMaterial
    material_id = 6
    block = ' 193  187  184  174'
    isotopes = 'pseudo_I3'
  []
  [ref]
    type = MixedNeutronicsMaterial
    material_id = 7
    block = ' 217  216  215  214'
    isotopes = 'pseudo_REF'
  []
  [r1]
    type = MixedNeutronicsMaterial
    material_id = 8
    block = '  49   36   35   34'
    isotopes = 'pseudo_R1'
  []
  [r2]
    type = MixedNeutronicsMaterial
    material_id = 9
    block = '  61   59   60   58   54   55   56   53   46   47   48   57   52   45   39   40   41   42   43   50   44   37   38   51'
    isotopes = 'pseudo_R2'
  []
  [shd]
    type = MixedNeutronicsMaterial
    material_id = 10
    block = '  33   32'
    isotopes = 'pseudo_SHD'
  []
  [lp]
    type = MixedNeutronicsMaterial
    material_id = 11
    block = '   1    0'
    isotopes = 'pseudo_LP'
  []
  [l1]
    type = MixedNeutronicsMaterial
    material_id = 12
    block = '  31   29   30   28   24   25   26   23   16   17   18   19   27   22   15    9   10   11   12   13   20   14    6    7    8    21    5    4    3    2'
    isotopes = 'pseudo_L1'
  []
  [up]
    type = MixedNeutronicsMaterial
    material_id = 13
    block = ' 191  192  190  186  188  185  179  180  181  189  178  172  173  175  176  182  177  170  171  183'
    isotopes = 'pseudo_UP'
  []
  [u1]
    type = MixedNeutronicsMaterial
    material_id = 14
    block = ' 212  213  211  208  209  207  202  203  204  210  201  196  197  198  199  205  200  194  195  206'
    isotopes = 'pseudo_U1'
  []
  [u2]
    type = MixedNeutronicsMaterial
    material_id = 15
    block = ' 247  245  246  244  240  241  242  239  232  233  234  235  243  238  231  225  226  227  228  229  236  230  222  223  224   237  221  220  219  218'
    isotopes = 'pseudo_U2'
  []
[]

[PowerDensity]
  power_density_variable = power
  power = 60.0
[]

[VectorPostprocessors]
  [assembly_power_2d]
    type = ExtraIDIntegralVectorPostprocessor
    variable = 'power'
    id_name = 'assembly_id'
  []
  [axial_power]
    type = ExtraIDIntegralVectorPostprocessor
    variable = 'power'
    id_name = 'plane_id'
  []
  [assembly_power_3d]
    type = ExtraIDIntegralVectorPostprocessor
    variable = 'power'
    id_name = 'assembly_id plane_id'
  []
[]

[Outputs]
  file_base = moose_cmfd_2quad
  csv = true
  [console]
    type = Console
    outlier_variable_norms = false
  []
  [pgraph]
    type = PerfGraphOutput
    level = 2
  []
[]
