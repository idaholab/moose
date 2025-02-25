# MARVEL Model, model full core, 4 heat exchangers
# Secondary as a single pipe
# Primary coolant = NaK
# Secondary coolant = Pb
# 1/29/2024 - Lise Charlot

## Operating conditions
p_primary = 0.392e6 # Pa #ANS paper
power_reactor = 85e3 # W #ANS paper
T_in_sec = '${fparse 273.15 + 386}'
v_in_sec = '${fparse -0.0615}'

## Core data
pin_diameter = 0.03269
pod = 1.06
pitch = '${fparse pin_diameter * pod}'
number_of_heater = 36 ##XXX##
r_Zr_rod = 0.00289
pin_cladding_inner_radius = '${fparse 0.5* 1.25*0.0254}'
fuel_thickness = '${fparse pin_cladding_inner_radius -r_Zr_rod }'
pin_cladding_thickness = '${fparse 0.5 *pin_diameter - pin_cladding_inner_radius}'

## Reflector data
dz_LP = '${fparse (3.07125-0.9646) * 0.0254}'

### Core inlet
z_cold_leg = '${fparse -dz_LP}'
z_bottom_plate = '${fparse z_cold_leg + dz_LP}'
L_unheated_bottom = 0.2
L_heated = 0.51
L_unheated_top = 0.2
L_core = '${fparse  L_unheated_bottom + L_heated + L_unheated_top}'

### Barrel
Barrel_ID = '${fparse 9.896 * 0.0254}'

## Riser
z_riser_lower = '${fparse z_bottom_plate + L_core}'
L_riser_lower = '${fparse (5.6147 + 0.5 * 1.90645)* 0.0254}'
z_riser_upper = '${fparse z_riser_lower + L_riser_lower }'
L_riser_upper = '${fparse (16.76 + 0.455 * 2.095)* 0.0254}'
CR_drive_OD = '${fparse 0.76895 * 0.0254}'
A_riser_lower = '${fparse 0.25 * pi * (Barrel_ID^2 - pin_diameter^2 -4 *CR_drive_OD^2) }'
P_wet_riser_lower = '${fparse pi * (Barrel_ID + pin_diameter + 4 *CR_drive_OD^2 )}'
Dh_riser_lower = '${fparse 4 * A_riser_lower / P_wet_riser_lower}'
Central_pin_large_OD = '${fparse 2.161 * 0.0254}'
A_riser_upper = '${fparse 0.25 * pi * (Barrel_ID^2 - Central_pin_large_OD^2  -4 *CR_drive_OD^2)}'
P_wet_riser_upper = '${fparse pi * (Barrel_ID + Central_pin_large_OD + 4*CR_drive_OD)}'
Dh_riser_upper = '${fparse 4 * A_riser_upper / P_wet_riser_upper}'
L_UP = '${fparse 6.825 * 0.0254}'
V_upper_plenum = '${fparse A_riser_upper * L_UP}'
x_downcomer = '${fparse 26.39 *0.5 * 0.0254 }' #updated

## cold_leg hole center to bottom barrel + barrel length + barrel_bo
L_cold_to_hot_leg = '${fparse (70.07) * 0.0254}'

### Hot leg and distribution plenum
L_hot_leg = '${fparse x_downcomer - (0.5* Barrel_ID +9.7825*0.5 * 0.0254)}'
hot_leg_ID = '${fparse 3.64 * 0.0254}'
A_hot_leg = '${fparse 0.25 * pi *hot_leg_ID^2 }'
dist_plen_height = '${fparse 5.915*0.0254}'
D_dist_plen = '${fparse 42.315 * 0.0254}'
D_CR_drive_up = '${fparse 2.275 *0.0254}'
D_sec_up = '${fparse 9.3275 *0.0254}'
L_sec_up = '${fparse 1.72536 *0.0254}'
D_sec_mid = '${fparse 8.30375 *0.0254}'
L_sec_mid = '${fparse 2.02202 *0.0254}'
D_sec_low = '${fparse 5.0505 *0.0254}'
L_sec_low = '${fparse dist_plen_height - L_sec_mid -L_sec_up}'
V_hot_leg = '${fparse A_hot_leg * L_hot_leg}'
V_sec_dist_plen = '${fparse pi * 0.25 *(D_sec_up^2 *L_sec_up + D_sec_mid^2 *L_sec_mid + D_sec_low^2 *L_sec_low)}'
V_dist_plen = '${fparse (0.25 * pi* dist_plen_height * (D_dist_plen^2 - 4 *D_CR_drive_up^2) -4 * V_hot_leg - 4* V_sec_dist_plen) /4}'
A_dist_plen = '${fparse V_dist_plen / dist_plen_height}'

### IHX
### IHX Primary
L_primary_IHX_up = '${fparse (1.17+0.5+4.66*0.5) * 0.0254}' ##updated##
L_primary_IHX_low = '${fparse (7.837+4.66*0.5) * 0.0254}' ##updated##
L_primary_IHX_outlet = '${fparse (3.667) * 0.0254 +0.03173}' ##updated##
primary_IHX_downcomer_up_ID = '${fparse 10.02 * 0.0254}' ##updated
primary_IHX_downcomer_low_ID = '${fparse 6.065 * 0.0254}' ##updated
secondary_IHX_pipe_OD = '${fparse 5.563 * 0.0254}'
secondary_IHX_pipe_ID = '${fparse 5.295 * 0.0254}'
secondary_IHX_liner_OD = '${fparse 3.5 * 0.0254}'
IHX_thickness = '${fparse 0.5*(secondary_IHX_pipe_OD-secondary_IHX_pipe_ID)}'
A_HX_primary_up = '${fparse pi/4. *( primary_IHX_downcomer_up_ID^2  - secondary_IHX_pipe_OD^2)}'
P_wet_primary_up = '${fparse pi *( primary_IHX_downcomer_up_ID + secondary_IHX_pipe_OD)}'
Dh_HX_primary_up = '${fparse 4 * A_HX_primary_up/P_wet_primary_up}'
P_hf_primary_up = '${fparse pi * secondary_IHX_pipe_OD}'
A_HX_primary_low = '${fparse pi/4 *( primary_IHX_downcomer_low_ID^2  - secondary_IHX_pipe_OD^2)}'
P_wet_primary_low = '${fparse pi *( primary_IHX_downcomer_low_ID + secondary_IHX_pipe_OD)}'
Dh_HX_primary_low = '${fparse 4 * A_HX_primary_low/P_wet_primary_low}'
P_hf_primary_low = '${fparse pi * secondary_IHX_pipe_OD}'
A_HX_secondary = '${fparse pi/4 *( secondary_IHX_pipe_ID^2  - secondary_IHX_liner_OD^2)}'
P_wet_secondary = '${fparse pi *( secondary_IHX_pipe_ID + secondary_IHX_liner_OD)}'
Dh_HX_secondary = '${fparse 4 * A_HX_secondary/P_wet_secondary}'
P_hf_secondary = '${fparse pi * secondary_IHX_pipe_ID}'

### Downcomer
#pipe1
L_downcomer_pipe1 = '${fparse 40.7225 * 0.0254}'
## Diameter for all downcomer pipes and cold leg, 2 in SCH40
downcomer_pipe_ID = '${fparse 1.88097 * 0.0254}'
downcomer_pipe_OD = '${fparse 2.1658 * 0.0254}'
downcomer_pipe_thickness = '${fparse 0.5 *(downcomer_pipe_OD - downcomer_pipe_ID)}'
#90 degree elbow pipe_ID 2in pipe
elbow_center_to_end = '${fparse 2.73 * 0.0254}'
L_cold_leg = '${fparse 6.3063 * 0.0254 + elbow_center_to_end}'

##################
# Parameter calcs
##################
D_LP = '${fparse 9.21375 * 0.0254}'
FA_LP = '${fparse 0.25 * pi * D_LP}'
L_LP = '${fparse 2*dz_LP}'
V_LP = '${fparse L_LP * FA_LP}'
FA_core = 0.00891407 #m2 Value from SCM
P_wet_core = 4.67831 #m Value from SCM
Dh_core = '${fparse 4 * FA_core/P_wet_core}'
P_hf_core = '${fparse pi * number_of_heater*pin_diameter}'
surface_density_core = '${fparse P_hf_core / FA_core}'
z_hot_leg = '${fparse L_cold_to_hot_leg + z_cold_leg}'
A_downcomer_pipe = '${fparse pi * 0.25 * downcomer_pipe_ID^2}'
Ihx_reducer = '${fparse 0.83447 * 0.0254}' ### Assumption
L_pipe_up = '${fparse elbow_center_to_end + Ihx_reducer}'
elbow45_center_to_end = '${fparse 1.25125 * 0.0254}'
L_downcomer_p1 = '${fparse L_downcomer_pipe1 + 2 * elbow_center_to_end}'
L_downcomer_h1 = '${fparse 3.8766 * 0.0254  + elbow45_center_to_end + elbow_center_to_end}'
L_downcomer_h2 = '${fparse 5.46 * 0.0254 +2 * elbow45_center_to_end }'
L_downcomer_h3 = '${fparse 1.6016 * 0.0254 + elbow45_center_to_end + elbow_center_to_end}'
dx_downcomer_h3 = '${fparse sqrt(2.)/2.* L_downcomer_h2}'
L_downcomer = '${fparse L_downcomer_p1 + L_pipe_up}'
z_downcomer = '${fparse z_cold_leg + L_downcomer}'
z_downcomer_h = '${fparse z_downcomer - L_pipe_up}'
dist_plen_dz = '${fparse 0.5 * dist_plen_height }'
K_loss = 0.2
##
[GlobalParams]
  global_init_P = 1.97e5
  global_init_V = 0.0001
  global_init_T = ${T_in_sec}
  supg_max = false
  pspg = true
  p_order = 1
  scaling_factor_var = '1. 1e-3 1e-5'
[]

[EOS]
  [eos]
    type = PTFunctionsEOS
    rho = rho_NaK
    cp = cp_NaK
    mu = mu_NaK
    k = k_NaK
    eos_test = true
    T_min = 300
    T_max = 1100
  []
  [eos_Pb]
    type = PTFunctionsEOS
    rho = rho_Pb
    cp = cp_Pb
    mu = mu_Pb
    k = k_Pb
    eos_test = true
    T_min = 300
    T_max = 1000
  []
  [eos_He]
    type = HeEquationOfState
  []
[]

[Functions]
  [T_in] # Pb inlet temperature
    type = PiecewiseLinear
    x = '0       1.e6'
    y = '${T_in_sec}     ${T_in_sec}'
  []
  [v_in] #  Pb inlet velocity
    type = PiecewiseLinear
    x = '0      1.e6'
    y = '1      1'
    scale_factor = ${v_in_sec}
  []
  ##########################
  [rho_NaK]
    type = PiecewiseLinear
    x = '323.15    373.15    423.15    473.15    523.15    573.15    623.15    673.15    723.15    773.15
823.15    873.15    923.15    973.15    1023.15'
    y = '866.9    855.6    844.2    832.7    821.1    809.4    797.6    785.7    773.7    761.6
749.5    737.2    724.9    712.5    700.1'
  []
  [mu_NaK]
    type = PiecewiseLinear
    x = '323.15    373.15    423.15    473.15    523.15    573.15    623.15    673.15    723.15    773.15
823.15    873.15    923.15    973.15    1023.15'
    y = '0.0007004    0.0005333    0.0004326    0.0003663    0.0003198    0.0002856    0.0002595    0.0002389    0.0002145    0.0001964
0.0001816    0.0001693    0.0001589    0.0001499    0.0001422'
  []
  [k_NaK]
    type = PiecewiseLinear
    x = '293.15    373.15    823.15'
    y = '21.8    23.2    26.2'
  []
  [cp_NaK]
    type = PiecewiseLinear
    x = '293.15    373.15    823.15'
    y = '971.34    937.84    870.85'
  []
  [rho_Pb]
    type = PiecewiseLinear
    x = '273 1000'
    y = '11091.6965 10161.5'
  []
  [mu_Pb]
    type = PiecewiseLinear
    x = '300 400    500    600    700    800    900    1000'
    y = '0.016052675 0.006586632 0.003859517 0.002702585 0.002095275 0.001731161 0.001492302 0.001325172'
  []
  [k_Pb]
    type = PiecewiseLinear
    x = '300 400    500    600    700    800    900    1000'
    y = '12.5 13.6 14.7 15.8 16.9 18 19.1 20.2'
  []
  [cp_Pb]
    type = PiecewiseLinear
    x = '300 400    500    600    700    800    900    1000'
    y = '145.0134937 148.772664 148.899125 147.7932827 146.2693389 144.660062 143.1178475 141.717'
  []
  [power_shape]
    type = PiecewiseLinear
    x = '0  ${fparse L_core} '
    y = '${fparse 1} ${fparse 1}'
    axis = x
  []
[]

[MaterialProperties]
  [ss-mat]
    type = HeatConductionMaterialProps
    k = 16.2
    Cp = 500
    rho = 7.946e3
  []
  [Zr]
    type = HeatConductionMaterialProps
    k = 22
    Cp = 270
    rho = 6490
  []
  [UZrH]
    type = HeatConductionMaterialProps
    k = 18.1
    Cp = 3100.35
    rho = 6752.35
  []
[]

[ComponentInputParameters]
  [SSpipe]
    type = PBPipeParameters
    eos = eos
    HS_BC_type = Adiabatic
    material_wall = ss-mat
    roughness = 1.e-6
    n_elems = 10
    n_wall_elems = 1
    wall_thickness = ${downcomer_pipe_thickness}
  []
[]

[Components]
  [reactor]
    type = ReactorPower
    # initial_power = 'core_power'
    initial_power = ${power_reactor}
    # initial_power = 0.0
  []
  [lower_plenum]
    type = PBVolumeBranch
    center = '0 0 ${z_cold_leg}'
    inputs = 'downcomer1_p2(out) downcomer2_p2(out) downcomer3_p2(out) downcomer4_p2(out)'
    outputs = 'core(in)'
    K = '${K_loss}  ${K_loss} ${K_loss} ${K_loss} ${K_loss}'
    Area = ${FA_LP}
    volume = ${V_LP}
    width = ${D_LP}
    height = ${L_LP}
    initial_P = ${p_primary}
    eos = eos
  []
  [core] # core channel
    type = PBCoreChannel
    position = '0 0 ${z_bottom_plate}'
    orientation = '0 0 1'
    A = ${FA_core}
    Dh = ${Dh_core}
    length = '${L_core}'
    n_elems = 50
    eos = eos
    power_shape_function = power_shape
    power_fraction = '0.0 1.0 0.0'
    HTC_geometry_type = Bundle
    WF_geometry_type = Pipe
    WF_user_option = ChengTodreas
    PoD = '${fparse pitch/pin_diameter}'
    HoD = 100
    HT_surface_area_density = ${surface_density_core}
    HTC_user_option = Default
    dim_hs = 1
    name_of_hs = 'Zr fuel clad'
    Ts_init = ${T_in_sec}
    n_heatstruct = 3
    fuel_type = cylinder
    material_hs = 'Zr UZrH ss-mat'
    width_of_hs = '${r_Zr_rod} ${fuel_thickness} ${pin_cladding_thickness}'
    elem_number_of_hs = '2 4  2'
    # overlap_coupled = true
    # overlap_pp = pressure_grad_target
  []
  [j3]
    type = PBSingleJunction
    eos = eos
    inputs = 'core(out)'
    outputs = 'riser_lower(in)'
    K = '${K_loss}'
  []
  [riser_lower] # lower riser
    type = PBOneDFluidComponent
    orientation = '0 0 1'
    position = '0 0 ${z_riser_lower}'
    eos = eos
    length = ${L_riser_lower}
    Dh = ${Dh_riser_lower}
    A = ${A_riser_lower}
    n_elems = 5
  []
  [j4]
    type = PBSingleJunction
    eos = eos
    inputs = 'riser_lower(out)'
    outputs = 'riser_upper(in)'
    K = 0
  []
  [riser_upper]
    type = PBOneDFluidComponent
    orientation = '0 0 1'
    position = '0 0 ${z_riser_upper}'
    eos = eos
    length = ${L_riser_upper}
    Dh = ${Dh_riser_upper}
    A = ${A_riser_upper}
    n_elems = 5
  []
  [upper_plenum] # upper riser
    type = PBLiquidVolume
    center = '0 0 ${fparse z_hot_leg}'
    eos = eos
    inputs = 'riser_upper(out)'
    outputs = 'pipe100(in) pipe200(in) pipe300(in) pipe400(in)'
    K = '0.0 ${K_loss} ${K_loss} ${K_loss} ${K_loss}'
    Area = ${A_riser_upper}
    volume = '${fparse V_upper_plenum}'
    width = ${Barrel_ID}
    height = ${L_UP}
    initial_level = '${fparse L_UP}'
    initial_T = ${T_in_sec}
    initial_P = ${p_primary}
    ambient_pressure = ${p_primary}
  []
  [pipe100] # upper riser to HX1 (primary_in)
    type = PBOneDFluidComponent
    input_parameters = SSpipe
    orientation = '1 0 0'
    position = '${fparse 0.5 *Barrel_ID} 0 ${z_hot_leg}'
    length = ${L_hot_leg}
    Dh = ${hot_leg_ID}
    A = ${A_hot_leg}
  []
  [pipe200]
    type = PBOneDFluidComponent
    input_parameters = SSpipe
    orientation = '-1 0 0'
    position = '${fparse -0.5 * Barrel_ID} 0 ${z_hot_leg}'
    length = ${L_hot_leg}
    Dh = ${hot_leg_ID}
    A = ${A_hot_leg}
  []
  [pipe300]
    type = PBOneDFluidComponent
    input_parameters = SSpipe
    orientation = '0 1 0'
    position = '0 ${fparse 0.5 * Barrel_ID}  ${z_hot_leg}'
    length = ${L_hot_leg}
    Dh = ${hot_leg_ID}
    A = ${A_hot_leg}
  []
  [pipe400]
    type = PBOneDFluidComponent
    input_parameters = SSpipe
    orientation = ' 0 -1  0'
    position = ' 0 ${fparse -0.5 * Barrel_ID}  ${z_hot_leg}'
    length = ${L_hot_leg}
    Dh = ${hot_leg_ID}
    A = ${A_hot_leg}
  []
  [dist_plenum_1]
    type = PBVolumeBranch
    center = '${fparse x_downcomer} 0 ${z_hot_leg}'
    eos = eos
    inputs = 'pipe100(out)'
    outputs = 'HX1_up(primary_in)'
    K = '${fparse K_loss} 0'
    Area = '${A_dist_plen}'
    volume = '${V_dist_plen}'
    width = '${fparse 0.5 * (downcomer_pipe_ID + primary_IHX_downcomer_low_ID)}'
    height = ${dist_plen_height}
  []
  [HX1_up]
    type = PBHeatExchanger
    eos = eos
    eos_secondary = eos_Pb
    position = '${x_downcomer} 0 ${fparse z_hot_leg - dist_plen_dz}'
    orientation = '0 0 -1'
    A = ${A_HX_primary_up}
    A_secondary = ${A_HX_secondary}
    Dh = ${Dh_HX_primary_up}
    Dh_secondary = ${Dh_HX_secondary}
    length = ${L_primary_IHX_up}
    length_secondary = ${L_primary_IHX_up}
    n_elems = 5
    initial_V_secondary = -0.0001
    disp_mode = -1
    HTC_geometry_type = Pipe
    HTC_geometry_type_secondary = Pipe
    HT_surface_area_density = '${fparse P_hf_primary_up/A_HX_primary_up}'
    HT_surface_area_density_secondary = '${fparse P_hf_secondary/A_HX_secondary}'
    heat_transfer_area_error_tolerance = 0.002
    Twall_init = ${T_in_sec}
    HX_type = countercurrent
    hs_type = cylinder #Plate
    wall_thickness = ${IHX_thickness}
    radius_i = '${fparse 0.5*secondary_IHX_pipe_ID}'
    dim_wall = 2
    material_wall = ss-mat
    n_wall_elems = 2
  []
  [j1_hx1]
    type = PBSingleJunction
    eos = eos
    inputs = 'HX1_up(primary_out)'
    outputs = 'HX1_low(primary_in)'
    K = 0.0
  []
  [HX1_low]
    type = PBHeatExchanger
    eos = eos
    eos_secondary = eos_Pb
    position = '${x_downcomer} 0 ${fparse z_hot_leg - dist_plen_dz- L_primary_IHX_up}'
    orientation = '0 0 -1'
    A = ${A_HX_primary_low}
    A_secondary = ${A_HX_secondary}
    Dh = ${Dh_HX_primary_low}
    Dh_secondary = ${Dh_HX_secondary}
    length = ${L_primary_IHX_low}
    length_secondary = ${L_primary_IHX_low}
    n_elems = 20
    initial_V_secondary = -0.0001
    disp_mode = -1
    HTC_geometry_type = Pipe
    HTC_geometry_type_secondary = Pipe
    HT_surface_area_density = '${fparse P_hf_primary_low/A_HX_primary_low}'
    HT_surface_area_density_secondary = '${fparse P_hf_secondary/A_HX_secondary}'
    heat_transfer_area_error_tolerance = 0.002
    Twall_init = ${T_in_sec}
    HX_type = countercurrent
    hs_type = cylinder #Plate
    wall_thickness = ${IHX_thickness}
    radius_i = '${fparse 0.5*secondary_IHX_pipe_ID}'
    dim_wall = 2
    material_wall = ss-mat
    n_wall_elems = 2
  []
  [HX1S_inlet] # set HX secondary side temperature and velocity (flow rate)
    type = PBTDJ
    eos = eos_Pb
    T_fn = T_in
    v_fn = v_in
    input = 'HX1_low(secondary_in)'
  []
  [HX1S_outlet]
    type = PBTDV
    eos = eos_Pb
    p_bc = 1.00E+05
    input = 'HX1_up(secondary_out)'
  []
  [j1_hxs]
    type = PBSingleJunction
    eos = eos_Pb
    inputs = 'HX1_low(secondary_out)'
    outputs = 'HX1_up(secondary_in)'
    K = 0
  []
  [j1_hx_to_p]
    type = PBVolumeBranch
    center = '${x_downcomer} 0 ${fparse z_downcomer + 0.5 * L_primary_IHX_outlet}'
    eos = eos
    inputs = 'HX1_low(primary_out)'
    outputs = 'downcomer1_pipe_up(in)'
    K = '${fparse K_loss} 0'
    Area = '${fparse 0.5 *(A_downcomer_pipe + pi * 0.25 *primary_IHX_downcomer_low_ID^2)}'
    volume = '${fparse 0.5 * (A_downcomer_pipe + pi * 0.25 *primary_IHX_downcomer_low_ID^2) * L_primary_IHX_outlet}'
    width = '${fparse 0.5 * (downcomer_pipe_ID + primary_IHX_downcomer_low_ID)}'
    height = ${L_primary_IHX_outlet}
  []
  [downcomer1_pipe_up] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 0 -1'
    position = '${x_downcomer} 0 ${fparse z_downcomer}'
    length = '${fparse L_pipe_up}'
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j1_1_to_h]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer1_pipe_up(out)'
    outputs = 'downcomer1_h1(in)'
    K = '${fparse K_loss}'
  []
  [downcomer1_h1] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 1 0'
    position = '${x_downcomer} 0 ${z_downcomer_h}'
    length = ${L_downcomer_h1}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j1_h1_to_h2]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer1_h1(out)'
    outputs = 'downcomer1_h2(in)'
    K = '${fparse K_loss}'
  []
  [downcomer1_h2] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '-1 1 0'
    position = '${x_downcomer}  ${L_downcomer_h1} ${z_downcomer_h}'
    length = ${L_downcomer_h2}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j1_h2_to_h3]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer1_h2(out)'
    outputs = 'downcomer1_h3(in)'
    K = '${fparse K_loss}'
  []
  [downcomer1_h3] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '-1 0 0'
    position = '${fparse x_downcomer -dx_downcomer_h3}  ${fparse L_downcomer_h1 +dx_downcomer_h3} ${z_downcomer_h}'
    length = ${L_downcomer_h3}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j1_h3_to_p1]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer1_h3(out)'
    outputs = 'downcomer1_p1(in)'
    K = '${fparse K_loss}'
  []
  [downcomer1_p1] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 0 -1'
    position = '${fparse x_downcomer -dx_downcomer_h3 -L_downcomer_h3}  ${fparse L_downcomer_h1 +dx_downcomer_h3} ${z_downcomer_h}'
    length = ${L_downcomer_p1}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j1_p1_to_p2]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer1_p1(out)'
    outputs = 'downcomer1_p2(in)'
    K = '${fparse K_loss}'
  []
  [downcomer1_p2] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 -1 0'
    position = '${fparse x_downcomer -dx_downcomer_h3 -L_downcomer_h3}  ${fparse L_downcomer_h1 +dx_downcomer_h3} ${fparse z_downcomer_h - L_downcomer_p1}'
    length = ${L_cold_leg}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [dist_plenum_2]
    type = PBVolumeBranch
    center = '${fparse -x_downcomer} 0 ${z_hot_leg}'
    eos = eos
    inputs = 'pipe200(out)'
    outputs = 'HX2_up(primary_in)'
    K = '${fparse K_loss} 0'
    Area = '${A_dist_plen}'
    volume = '${V_dist_plen}'
    width = '${fparse 0.5 * (downcomer_pipe_ID + primary_IHX_downcomer_low_ID)}'
    height = ${dist_plen_height}
  []
  [HX2_up]
    type = PBHeatExchanger
    eos = eos
    eos_secondary = eos_Pb
    position = '${fparse -x_downcomer} 0 ${fparse z_hot_leg - dist_plen_dz}'
    orientation = '0 0 -1'
    A = ${A_HX_primary_up}
    A_secondary = ${A_HX_secondary}
    Dh = ${Dh_HX_primary_up}
    Dh_secondary = ${Dh_HX_secondary}
    length = ${L_primary_IHX_up}
    length_secondary = ${L_primary_IHX_up}
    n_elems = 5
    initial_V_secondary = -0.0001
    disp_mode = -1
    HTC_geometry_type = Pipe
    HTC_geometry_type_secondary = Pipe
    HT_surface_area_density = '${fparse P_hf_primary_up/A_HX_primary_up}'
    HT_surface_area_density_secondary = '${fparse P_hf_secondary/A_HX_secondary}'
    heat_transfer_area_error_tolerance = 0.002
    Twall_init = ${T_in_sec}
    HX_type = countercurrent
    hs_type = cylinder #Plate
    wall_thickness = ${IHX_thickness}
    radius_i = '${fparse 0.5*secondary_IHX_pipe_ID}'
    dim_wall = 2
    material_wall = ss-mat
    n_wall_elems = 2
  []
  [j2_hx1]
    type = PBSingleJunction
    eos = eos
    inputs = 'HX2_up(primary_out)'
    outputs = 'HX2_low(primary_in)'
    K = 0.0
  []
  [HX2_low]
    type = PBHeatExchanger
    eos = eos
    eos_secondary = eos_Pb
    position = '${fparse -x_downcomer} 0 ${fparse z_hot_leg - dist_plen_dz- L_primary_IHX_up}'
    orientation = '0 0 -1'
    A = ${A_HX_primary_low}
    A_secondary = ${A_HX_secondary}
    Dh = ${Dh_HX_primary_low}
    Dh_secondary = ${Dh_HX_secondary}
    length = ${L_primary_IHX_low}
    length_secondary = ${L_primary_IHX_low}
    n_elems = 20
    initial_V_secondary = -0.0001
    disp_mode = -1
    HTC_geometry_type = Pipe
    HTC_geometry_type_secondary = Pipe
    HT_surface_area_density = '${fparse P_hf_primary_low/A_HX_primary_low}'
    HT_surface_area_density_secondary = '${fparse P_hf_secondary/A_HX_secondary}'
    heat_transfer_area_error_tolerance = 0.002
    Twall_init = ${T_in_sec}
    HX_type = countercurrent
    hs_type = cylinder #Plate
    wall_thickness = ${IHX_thickness}
    radius_i = '${fparse 0.5*secondary_IHX_pipe_ID}'
    dim_wall = 2
    material_wall = ss-mat
    n_wall_elems = 2
  []
  [HX2S_inlet] # set HX secondary side temperature and velocity (flow rate)
    type = PBTDJ
    eos = eos_Pb
    T_bc = ${T_in_sec}
    v_bc = ${v_in_sec}
    input = 'HX2_low(secondary_in)'
  []
  [HX2S_outlet]
    type = PBTDV
    eos = eos_Pb
    p_bc = 1.00E+05
    input = 'HX2_up(secondary_out)'
  []
  [j2_hxs]
    type = PBSingleJunction
    eos = eos_Pb
    inputs = 'HX2_low(secondary_out)'
    outputs = 'HX2_up(secondary_in)'
    K = 0.0
  []
  [j2_hx_to_p]
    type = PBVolumeBranch
    center = '${fparse -x_downcomer} 0 ${fparse z_downcomer + 0.5 * L_primary_IHX_outlet}'
    eos = eos
    inputs = 'HX2_low(primary_out)'
    outputs = 'downcomer2_pipe_up(in)'
    K = '${fparse K_loss} 0'
    Area = '${fparse 0.5 *(A_downcomer_pipe + pi * 0.25 *primary_IHX_downcomer_low_ID^2)}'
    volume = '${fparse 0.5 * (A_downcomer_pipe + pi * 0.25 *primary_IHX_downcomer_low_ID^2) * L_primary_IHX_outlet}'
    width = '${fparse 0.5 * (downcomer_pipe_ID + primary_IHX_downcomer_low_ID)}'
    height = ${L_primary_IHX_outlet}
  []
  [downcomer2_pipe_up] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 0 -1'
    position = '${fparse -x_downcomer} 0 ${fparse z_downcomer}'
    length = '${fparse L_pipe_up}'
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j2_1_to_h]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer2_pipe_up(out)'
    outputs = 'downcomer2_h1(in)'
    K = '${fparse K_loss}'
  []
  [downcomer2_h1] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 -1 0'
    position = '${fparse -x_downcomer} 0 ${z_downcomer_h}'
    length = ${L_downcomer_h1}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j2_h1_to_h2]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer2_h1(out)'
    outputs = 'downcomer2_h2(in)'
    K = '${fparse K_loss}'
  []
  [downcomer2_h2] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '1 -1 0'
    position = '${fparse -x_downcomer}  ${fparse -L_downcomer_h1} ${z_downcomer_h}'
    length = ${L_downcomer_h2}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j2_h2_to_h3]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer2_h2(out)'
    outputs = 'downcomer2_h3(in)'
    K = '${fparse K_loss}'
  []
  [downcomer2_h3] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '1 0 0'
    position = '${fparse -x_downcomer +dx_downcomer_h3}  ${fparse -L_downcomer_h1 -dx_downcomer_h3} ${z_downcomer_h}'
    length = ${L_downcomer_h3}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j2_h3_to_p1]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer2_h3(out)'
    outputs = 'downcomer2_p1(in)'
    K = '${fparse K_loss}'
  []
  [downcomer2_p1] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 0 -1'
    position = '${fparse -x_downcomer +dx_downcomer_h3 +L_downcomer_h3}  ${fparse -L_downcomer_h1 -dx_downcomer_h3} ${z_downcomer_h}'
    length = ${L_downcomer_p1}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j2_p1_to_p2]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer2_p1(out)'
    outputs = 'downcomer2_p2(in)'
    K = '${fparse K_loss}'
  []
  [downcomer2_p2] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 1 0'
    position = '${fparse -x_downcomer +dx_downcomer_h3 +L_downcomer_h3}  ${fparse -L_downcomer_h1 -dx_downcomer_h3} ${fparse z_downcomer_h - L_downcomer_p1}'
    length = ${L_cold_leg}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [dist_plenum_3]
    type = PBVolumeBranch
    center = '0 ${fparse x_downcomer} ${z_hot_leg}'
    eos = eos
    inputs = 'pipe300(out)'
    outputs = 'HX3_up(primary_in)'
    K = '${fparse K_loss} 0'
    Area = '${A_dist_plen}'
    volume = '${V_dist_plen}'
    width = '${fparse 0.5 * (downcomer_pipe_ID + primary_IHX_downcomer_low_ID)}'
    height = ${dist_plen_height}
  []
  [HX3_up]
    type = PBHeatExchanger
    eos = eos
    eos_secondary = eos_Pb
    position = '0 ${x_downcomer}  ${fparse z_hot_leg - dist_plen_dz}'
    orientation = '0 0 -1'
    A = ${A_HX_primary_up}
    A_secondary = ${A_HX_secondary}
    Dh = ${Dh_HX_primary_up}
    Dh_secondary = ${Dh_HX_secondary}
    length = ${L_primary_IHX_up}
    length_secondary = ${L_primary_IHX_up}
    n_elems = 5
    initial_V_secondary = -0.0001
    disp_mode = -1
    HTC_geometry_type = Pipe
    HTC_geometry_type_secondary = Pipe
    HT_surface_area_density = '${fparse P_hf_primary_up/A_HX_primary_up}'
    HT_surface_area_density_secondary = '${fparse P_hf_secondary/A_HX_secondary}'
    heat_transfer_area_error_tolerance = 0.002
    Twall_init = ${T_in_sec}
    HX_type = countercurrent
    hs_type = cylinder #Plate
    wall_thickness = ${IHX_thickness}
    radius_i = '${fparse 0.5*secondary_IHX_pipe_ID}'
    dim_wall = 2
    material_wall = ss-mat
    n_wall_elems = 2
  []
  [j3_hx1]
    type = PBSingleJunction
    eos = eos
    inputs = 'HX3_up(primary_out)'
    outputs = 'HX3_low(primary_in)'
    # K = '${fparse K_loss}'
  []
  [HX3_low]
    type = PBHeatExchanger
    eos = eos
    eos_secondary = eos_Pb
    position = '0 ${x_downcomer}  ${fparse z_hot_leg - dist_plen_dz- L_primary_IHX_up}'
    orientation = '0 0 -1'
    A = ${A_HX_primary_low}
    A_secondary = ${A_HX_secondary}
    Dh = ${Dh_HX_primary_low}
    Dh_secondary = ${Dh_HX_secondary}
    length = ${L_primary_IHX_low}
    length_secondary = ${L_primary_IHX_low}
    n_elems = 20
    initial_V_secondary = -0.0001
    disp_mode = -1
    HTC_geometry_type = Pipe
    HTC_geometry_type_secondary = Pipe
    HT_surface_area_density = '${fparse P_hf_primary_low/A_HX_primary_low}'
    HT_surface_area_density_secondary = '${fparse P_hf_secondary/A_HX_secondary}'
    heat_transfer_area_error_tolerance = 0.002
    Twall_init = ${T_in_sec}
    HX_type = countercurrent
    hs_type = cylinder #Plate
    wall_thickness = ${IHX_thickness}
    radius_i = '${fparse 0.5*secondary_IHX_pipe_ID}'
    dim_wall = 2
    material_wall = ss-mat
    n_wall_elems = 2
  []
  [HX3S_inlet] # set HX secondary side temperature and velocity (flow rate)
    type = PBTDJ
    eos = eos_Pb
    T_bc = ${T_in_sec}
    v_bc = ${v_in_sec}
    input = 'HX3_low(secondary_in)'
  []
  [HX3S_outlet]
    type = PBTDV
    eos = eos_Pb
    p_bc = 1.00E+05
    input = 'HX3_up(secondary_out)'
  []
  [j3_hxs]
    type = PBSingleJunction
    eos = eos_Pb
    inputs = 'HX3_low(secondary_out)'
    outputs = 'HX3_up(secondary_in)'
    K = '${fparse K_loss}'
  []
  [j3_hx_to_p]
    type = PBVolumeBranch
    center = '0 ${x_downcomer} ${fparse z_downcomer + 0.5 * L_primary_IHX_outlet}'
    eos = eos
    inputs = 'HX3_low(primary_out)'
    outputs = 'downcomer3_pipe_up(in)'
    K = '${fparse K_loss} 0'
    Area = '${fparse 0.5 *(A_downcomer_pipe + pi * 0.25 *primary_IHX_downcomer_low_ID^2)}'
    volume = '${fparse 0.5 * (A_downcomer_pipe + pi * 0.25 *primary_IHX_downcomer_low_ID^2) * L_primary_IHX_outlet}'
    width = '${fparse 0.5 * (downcomer_pipe_ID + primary_IHX_downcomer_low_ID)}'
    height = ${L_primary_IHX_outlet}
  []
  [downcomer3_pipe_up] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 0 -1'
    position = '0 ${x_downcomer}  ${fparse z_downcomer}'
    length = '${fparse L_pipe_up}'
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j3_1_to_h]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer3_pipe_up(out)'
    outputs = 'downcomer3_h1(in)'
    K = '${fparse K_loss}'
  []
  [downcomer3_h1] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '-1 0  0'
    position = '0 ${x_downcomer} ${z_downcomer_h}'
    length = ${L_downcomer_h1}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j3_h1_to_h2]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer3_h1(out)'
    outputs = 'downcomer3_h2(in)'
    K = '${fparse K_loss}'
  []
  [downcomer3_h2] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '-1 -1 0'
    position = ' ${fparse -L_downcomer_h1} ${x_downcomer}  ${z_downcomer_h}'
    length = ${L_downcomer_h2}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j3_h2_to_h3]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer3_h2(out)'
    outputs = 'downcomer3_h3(in)'
    K = '${fparse K_loss}'
  []
  [downcomer3_h3] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 -1 0'
    position = '${fparse -L_downcomer_h1 -dx_downcomer_h3} ${fparse x_downcomer -dx_downcomer_h3}   ${z_downcomer_h}'
    length = ${L_downcomer_h3}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j3_h3_to_p1]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer3_h3(out)'
    outputs = 'downcomer3_p1(in)'
    K = '${fparse K_loss}'
  []
  [downcomer3_p1] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 0 -1'
    position = '${fparse -L_downcomer_h1 -dx_downcomer_h3} ${fparse x_downcomer -dx_downcomer_h3 -L_downcomer_h3} ${z_downcomer_h}'
    length = ${L_downcomer_p1}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j3_p1_to_p2]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer3_p1(out)'
    outputs = 'downcomer3_p2(in)'
    K = '${fparse K_loss}'
  []
  [downcomer3_p2] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '1 0 0'
    position = ' ${fparse -L_downcomer_h1 -dx_downcomer_h3} ${fparse x_downcomer -dx_downcomer_h3 -L_downcomer_h3}  ${fparse z_downcomer_h - L_downcomer_p1}'
    length = ${L_cold_leg}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [dist_plenum_4]
    type = PBVolumeBranch
    center = '0 ${fparse -x_downcomer} ${z_hot_leg}'
    eos = eos
    inputs = 'pipe400(out)'
    outputs = 'HX4_up(primary_in)'
    K = '${fparse K_loss} 0'
    Area = '${A_dist_plen}'
    volume = '${V_dist_plen}'
    width = '${fparse 0.5 * (downcomer_pipe_ID + primary_IHX_downcomer_low_ID)}'
    height = ${dist_plen_height}
  []
  [HX4_up]
    type = PBHeatExchanger
    eos = eos
    eos_secondary = eos_Pb
    position = '0 ${fparse -x_downcomer} ${fparse z_hot_leg - dist_plen_dz}'
    orientation = '0 0 -1'
    A = ${A_HX_primary_up}
    A_secondary = ${A_HX_secondary}
    Dh = ${Dh_HX_primary_up}
    Dh_secondary = ${Dh_HX_secondary}
    length = ${L_primary_IHX_up}
    length_secondary = ${L_primary_IHX_up}
    n_elems = 5
    initial_V_secondary = -0.0001
    disp_mode = -1
    HTC_geometry_type = Pipe
    HTC_geometry_type_secondary = Pipe
    HT_surface_area_density = '${fparse P_hf_primary_up/A_HX_primary_up}'
    HT_surface_area_density_secondary = '${fparse P_hf_secondary/A_HX_secondary}'
    heat_transfer_area_error_tolerance = 0.002
    Twall_init = ${T_in_sec}
    HX_type = countercurrent
    hs_type = cylinder #Plate
    wall_thickness = ${IHX_thickness}
    radius_i = '${fparse 0.5*secondary_IHX_pipe_ID}'
    dim_wall = 2
    material_wall = ss-mat
    n_wall_elems = 2
  []
  [j4_hx1]
    type = PBSingleJunction
    eos = eos
    inputs = 'HX4_up(primary_out)'
    outputs = 'HX4_low(primary_in)'
    # K = '${fparse K_loss}'
  []
  [HX4_low]
    type = PBHeatExchanger
    eos = eos
    eos_secondary = eos_Pb
    position = '0 ${fparse -x_downcomer}  ${fparse z_hot_leg - dist_plen_dz- L_primary_IHX_up}'
    orientation = '0 0 -1'
    A = ${A_HX_primary_low}
    A_secondary = ${A_HX_secondary}
    Dh = ${Dh_HX_primary_low}
    Dh_secondary = ${Dh_HX_secondary}
    length = ${L_primary_IHX_low}
    length_secondary = ${L_primary_IHX_low}
    n_elems = 20
    initial_V_secondary = -0.0001
    disp_mode = -1
    HTC_geometry_type = Pipe
    HTC_geometry_type_secondary = Pipe
    HT_surface_area_density = '${fparse P_hf_primary_low/A_HX_primary_low}'
    HT_surface_area_density_secondary = '${fparse P_hf_secondary/A_HX_secondary}'
    heat_transfer_area_error_tolerance = 0.002
    Twall_init = ${T_in_sec}
    HX_type = countercurrent
    hs_type = cylinder #Plate
    wall_thickness = ${IHX_thickness}
    radius_i = '${fparse 0.5*secondary_IHX_pipe_ID}'
    dim_wall = 2
    material_wall = ss-mat
    n_wall_elems = 2
  []
  [HX4S_inlet] # set HX secondary side temperature and velocity (flow rate)
    type = PBTDJ
    eos = eos_Pb
    T_bc = ${T_in_sec}
    v_bc = ${v_in_sec}
    input = 'HX4_low(secondary_in)'
  []
  [HX4S_outlet]
    type = PBTDV
    eos = eos_Pb
    p_bc = 1.00E+05
    input = 'HX4_up(secondary_out)'
  []
  [j4_hxs]
    type = PBSingleJunction
    eos = eos_Pb
    inputs = 'HX4_low(secondary_out)'
    outputs = 'HX4_up(secondary_in)'
    K = '${fparse K_loss}'
  []
  [j4_hx_to_p]
    type = PBVolumeBranch
    center = '0 ${fparse -x_downcomer} ${fparse z_downcomer + 0.5 * L_primary_IHX_outlet}'
    eos = eos
    inputs = 'HX4_low(primary_out)'
    outputs = 'downcomer4_pipe_up(in)'
    K = '${fparse K_loss} 0'
    Area = '${fparse 0.5 *(A_downcomer_pipe + pi * 0.25 *primary_IHX_downcomer_low_ID^2)}'
    volume = '${fparse 0.5 * (A_downcomer_pipe + pi * 0.25 *primary_IHX_downcomer_low_ID^2) * L_primary_IHX_outlet}'
    width = '${fparse 0.5 * (downcomer_pipe_ID + primary_IHX_downcomer_low_ID)}'
    height = ${L_primary_IHX_outlet}
  []
  [downcomer4_pipe_up] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 0 -1'
    position = '0 ${fparse -x_downcomer} ${fparse z_downcomer}'
    length = '${fparse L_pipe_up}'
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j4_1_to_h]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer4_pipe_up(out)'
    outputs = 'downcomer4_h1(in)'
    K = '${fparse K_loss}'
  []
  [downcomer4_h1] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '1 0 0'
    position = '0 ${fparse -x_downcomer} ${z_downcomer_h}'
    length = ${L_downcomer_h1}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j4_h1_to_h2]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer4_h1(out)'
    outputs = 'downcomer4_h2(in)'
    K = '${fparse K_loss}'
  []
  [downcomer4_h2] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '1 1 0'
    position = ' ${fparse L_downcomer_h1} ${fparse -x_downcomer} ${z_downcomer_h}'
    length = ${L_downcomer_h2}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j4_h2_to_h3]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer4_h2(out)'
    outputs = 'downcomer4_h3(in)'
    K = '${fparse K_loss}'
  []
  [downcomer4_h3] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 1 0'
    position = '${fparse L_downcomer_h1 +dx_downcomer_h3} ${fparse -x_downcomer +dx_downcomer_h3} ${z_downcomer_h}'
    length = ${L_downcomer_h3}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j4_h3_to_p1]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer4_h3(out)'
    outputs = 'downcomer4_p1(in)'
    K = '${fparse K_loss}'
  []
  [downcomer4_p1] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '0 0 -1'
    position = ' ${fparse L_downcomer_h1 +dx_downcomer_h3} ${fparse -x_downcomer +dx_downcomer_h3 +L_downcomer_h3} ${z_downcomer_h}'
    length = ${L_downcomer_p1}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
  [j4_p1_to_p2]
    type = PBSingleJunction
    eos = eos
    inputs = 'downcomer4_p1(out)'
    outputs = 'downcomer4_p2(in)'
    K = '${fparse K_loss}'
  []
  [downcomer4_p2] # downcomer
    type = PBPipe
    input_parameters = SSpipe
    orientation = '-1 0 0'
    position = '${fparse L_downcomer_h1 +dx_downcomer_h3} ${fparse -x_downcomer +dx_downcomer_h3 +L_downcomer_h3}   ${fparse z_downcomer_h - L_downcomer_p1}'
    length = ${L_cold_leg}
    Dh = ${downcomer_pipe_ID}
    A = ${A_downcomer_pipe}
  []
[]

[Postprocessors]
  [core_Pout]
    type = ComponentBoundaryVariableValue
    input = core(out)
    variable = pressure
  []
  [core_Pin]
    type = ComponentBoundaryVariableValue
    input = core(in)
    variable = pressure
  []
  [core_Tout]
    type = ComponentBoundaryVariableValue
    input = core(out)
    variable = temperature
  []
  [temperature_rise_core]
    type = ParsedPostprocessor
    pp_names = 'core_Tin core_Tout'
    expression = 'core_Tout - core_Tin'
  []
  [core_mdot_in]
    type = ComponentBoundaryFlow
    input = core(in)
  []
  [HX1_Tout]
    type = ComponentBoundaryVariableValue
    input = HX1_low(primary_out)
    variable = temperature
  []
  [HX1_Tin]
    type = ComponentBoundaryVariableValue
    input = HX1_up(primary_in)
    variable = temperature
  []
  [HX1_sec_mdot]
    type = ComponentBoundaryFlow
    input = HX1_up(secondary_out)
  []
  [HX1_STout]
    type = ComponentBoundaryVariableValue
    input = HX1_up(secondary_out)
    variable = temperature
  []
  [HX1_STin]
    type = ComponentBoundaryVariableValue
    input = HX1_low(secondary_in)
    variable = temperature
  []
  [core_energy]
    type = ComponentBoundaryEnergyBalance
    eos = eos
    input = 'core(in) core(out)'
  []
  [pressure_drop_SAM]
    type = ParsedPostprocessor
    pp_names = 'core_Pout core_Pin'
    expression = 'core_Pin - core_Pout'
  []
  [pressure_grad_SAM]
    type = ParsedPostprocessor
    pp_names = 'pressure_drop_SAM'
    expression = 'pressure_drop_SAM / ${L_core}'
  []
  ############## Pressure Gradient Calculation #############
  [pressure_grad_target]
    type = ParsedPostprocessor
    pp_names = 'core_delta_p_tgt'
    expression = 'core_delta_p_tgt / ${L_core}'
  []
  ##### INFO to send to SC
  [P_core_out]
    type = ComponentBoundaryVariableValue
    input = core(out)
    variable = pressure
  []
  [core_Tin]
    type = ComponentBoundaryVariableValue
    input = core(in)
    variable = temperature
  []
  [inlet_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'core_mdot_in'
    expression = 'abs(core_mdot_in/${FA_core})'
  []
  [inlet_mass_flux_final]
    type = ParsedPostprocessor
    pp_names = 'inlet_mass_flux'
    expression = 'max(inlet_mass_flux, 15.0)'
  []
  #####
  ##### Info received from subchannel
  [core_delta_p_tgt]
    type = Receiver
    default = 5.997923e+03
  []
  [core_power]
    type = Receiver
    default = ${power_reactor}
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  start_time = -10000
  end_time = 0
  dt = 0.2
  dtmin = 1e-5
  dtmax = 500

  [TimeStepper]
    type = IterationAdaptiveDT
    growth_factor = 1.2
    optimal_iterations = 8
    linear_iteration_ratio = 150
    dt = 0.1
    cutback_factor = 0.8
    cutback_factor_at_failure = 0.8
  []

  petsc_options_iname = '-ksp_gmres_restart -pc_type'
  petsc_options_value = '300 lu '
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 12

  l_tol = 1e-4
  l_max_its = 100

  [Quadrature]
    type = TRAP
    order = FIRST
  []
[]

[Outputs]
  csv = true
  perf_graph = true
  print_linear_residuals = false
  [out_displaced]
    type = Exodus
    use_displaced = true
    execute_on = 'initial timestep_end'
    sequence = false
    output_material_properties = true
    show_material_properties = 'Re friction'
  []

  [checkpoint]
    type = Checkpoint
    num_files = 1
  []

  [console]
    type = Console
    time_step_interval = 10
  []
[]

################################################################################
# A multiapp that couples SAM to subchannel
################################################################################
[MultiApps]
  [subchannel]
    type = FullSolveMultiApp
    input_files = 'marvel_core_v2.i'
    execute_on = 'timestep_end'
    positions = '0 0 0'
    app_type = SubChannelApp
    max_procs_per_app = 1
    output_in_position = true
    bounding_box_padding = '0 0 0.1'
  []
[]

[Transfers]
  [pressure_drop_transfer] # Get pressure drop to SAM from SCM
    type = MultiAppPostprocessorTransfer
    from_multi_app = subchannel
    from_postprocessor = total_pressure_drop_SC
    to_postprocessor = core_delta_p_tgt
    reduction_type = average
    execute_on = 'timestep_end'
  []
  [power_transfer] # Get Total power to SAM from SCM
    type = MultiAppPostprocessorTransfer
    from_multi_app = subchannel
    from_postprocessor = Total_power
    to_postprocessor = core_power
    reduction_type = average
    execute_on = 'timestep_end'
  []
  [mass_flux_tranfer] # Send mass_flux at the inlet of SAM core to SCM
    type = MultiAppPostprocessorTransfer
    to_multi_app = subchannel
    from_postprocessor = inlet_mass_flux_final
    to_postprocessor = report_mass_flux_inlet
    execute_on = 'timestep_end'
  []
  [outlet_pressure_tranfer] # Send pressure at the outlet of SAM core to SCM
    type = MultiAppPostprocessorTransfer
    to_multi_app = subchannel
    from_postprocessor = P_core_out
    to_postprocessor = report_pressure_outlet
    execute_on = 'timestep_end'
  []
  [inlet_temperature_transfer] # Send temperature at the inlet of SAM core to SCM
    type = MultiAppPostprocessorTransfer
    to_multi_app = subchannel
    from_postprocessor = core_Tin
    to_postprocessor = report_temperature_inlet
    execute_on = 'timestep_end'
  []
[]
