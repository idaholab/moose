#
# SLKKS two phase example for the BCC and SIGMA phases. The sigma phase contains
# multiple sublattices. Free energy from
# Jacob, Aurelie, Erwin Povoden-Karadeniz, and Ernst Kozeschnik. "Revised thermodynamic
# description of the Fe-Cr system based on an improved sublattice model of the sigma phase."
# Calphad 60 (2018): 16-28.
#
# In this suimulation we consider diffusion (Cahn-Hilliard) and phase transformation.
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Note: This input is currently in draft status. The free energies are in the wrong units.
# and the Allen-Cahn equations are not quite correct yet.
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 260
  ny = 1
  nz = 0
  xmin = -25
  xmax = 25
  ymin = -2.5
  ymax = 2.5
  elem_type = QUAD4
[]

[AuxVariables]
  [Fglobal]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxVariables]
  [eta2]
  []
[]

[AuxKernels]
  [eta2]
    type = ParsedAux
    variable = eta2
    function = 1-eta1
    args = eta1
  []
[]

[Variables]
  # order parameter
  [eta1]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []

  # solute concentration
  [cCr]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      # type = RandomIC
      # min = 0.34
      # max = 0.36
      type = FunctionIC
      function = (x+25)/500+0.3
    []
  []

  # sublattice concentrations
  [BCC_CR]
    initial_condition = 0.45
  []
  [SIGMA_0CR]
    initial_condition = 0.13
  []
  [SIGMA_1CR]
    initial_condition = 0.3
  []
  [SIGMA_2CR]
    initial_condition = 0.5
  []
[]

[Materials]
  # CALPHAD free energies
  [F_BCC_A2]
    type = DerivativeParsedMaterial
    f_name = F_BCC_A2
    outputs = exodus
    output_properties = F_BCC_A2
    function = 'BCC_FE:=1-BCC_CR; G := 8.3145*T*(1.0*if(BCC_CR > 1.0e-15,BCC_CR*log(BCC_CR),0) + '
               '1.0*if(BCC_FE > 1.0e-15,BCC_FE*log(BCC_FE),0) + 3.0*if(BCC_VA > '
               '1.0e-15,BCC_VA*log(BCC_VA),0))/(BCC_CR + BCC_FE) + 8.3145*T*if(T < '
               '548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - 932.5*BCC_CR*BCC_FE*BCC_VA + '
               '311.5*BCC_CR*BCC_VA - '
               '1043.0*BCC_FE*BCC_VA,-8.13674105561218e-49*T^15/(0.525599232981783*BCC_CR*BCC'
               '_FE*BCC_VA*(BCC_CR - BCC_FE) - 0.894055608820709*BCC_CR*BCC_FE*BCC_VA + '
               '0.298657718120805*BCC_CR*BCC_VA - BCC_FE*BCC_VA + 9.58772770853308e-13)^15 - '
               '4.65558036243985e-30*T^9/(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - '
               'BCC_FE) - 0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^9 - '
               '1.3485349181899e-10*T^3/(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^3 + 1 - '
               '0.905299382744392*(548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - '
               '932.5*BCC_CR*BCC_FE*BCC_VA + 311.5*BCC_CR*BCC_VA - 1043.0*BCC_FE*BCC_VA + '
               '1.0e-9)/T,if(T < -548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '932.5*BCC_CR*BCC_FE*BCC_VA - 311.5*BCC_CR*BCC_VA + '
               '1043.0*BCC_FE*BCC_VA,-8.13674105561218e-49*T^15/(-0.525599232981783*BCC_CR*BC'
               'C_FE*BCC_VA*(BCC_CR - BCC_FE) + 0.894055608820709*BCC_CR*BCC_FE*BCC_VA - '
               '0.298657718120805*BCC_CR*BCC_VA + BCC_FE*BCC_VA + 9.58772770853308e-13)^15 - '
               '4.65558036243985e-30*T^9/(-0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - '
               'BCC_FE) + 0.894055608820709*BCC_CR*BCC_FE*BCC_VA - 0.298657718120805*BCC_CR*BCC_VA + '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^9 - '
               '1.3485349181899e-10*T^3/(-0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - '
               'BCC_FE) + 0.894055608820709*BCC_CR*BCC_FE*BCC_VA - 0.298657718120805*BCC_CR*BCC_VA + '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^3 + 1 - '
               '0.905299382744392*(-548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '932.5*BCC_CR*BCC_FE*BCC_VA - 311.5*BCC_CR*BCC_VA + 1043.0*BCC_FE*BCC_VA + '
               '1.0e-9)/T,if(T > -548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '932.5*BCC_CR*BCC_FE*BCC_VA - 311.5*BCC_CR*BCC_VA + 1043.0*BCC_FE*BCC_VA & '
               '548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - 932.5*BCC_CR*BCC_FE*BCC_VA + '
               '311.5*BCC_CR*BCC_VA - 1043.0*BCC_FE*BCC_VA < '
               '0,-79209031311018.7*(-0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - '
               'BCC_FE) + 0.894055608820709*BCC_CR*BCC_FE*BCC_VA - 0.298657718120805*BCC_CR*BCC_VA + '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^5/T^5 - '
               '3.83095660520737e+42*(-0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA - 0.298657718120805*BCC_CR*BCC_VA + '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^15/T^15 - '
               '1.22565886734485e+72*(-0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA - 0.298657718120805*BCC_CR*BCC_VA + '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^25/T^25,if(T > '
               '548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - 932.5*BCC_CR*BCC_FE*BCC_VA + '
               '311.5*BCC_CR*BCC_VA - 1043.0*BCC_FE*BCC_VA & 548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - '
               'BCC_FE) - 932.5*BCC_CR*BCC_FE*BCC_VA + 311.5*BCC_CR*BCC_VA - 1043.0*BCC_FE*BCC_VA > '
               '0,-79209031311018.7*(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - '
               'BCC_FE) - 0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^5/T^5 - '
               '3.83095660520737e+42*(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - '
               'BCC_FE) - 0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^15/T^15 - '
               '1.22565886734485e+72*(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - '
               'BCC_FE) - 0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + '
               '9.58772770853308e-13)^25/T^25,0))))*log((2.15*BCC_CR*BCC_FE*BCC_VA - '
               '0.008*BCC_CR*BCC_VA + 2.22*BCC_FE*BCC_VA)*if(2.15*BCC_CR*BCC_FE*BCC_VA - '
               '0.008*BCC_CR*BCC_VA + 2.22*BCC_FE*BCC_VA <= 0,-1.0,1.0) + 1)/(BCC_CR + BCC_FE) + '
               '1.0*(BCC_CR*BCC_VA*if(T >= 298.15 & T < 2180.0,139250.0*1/T - 26.908*T*log(T) + '
               '157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,if(T >= 2180.0 & T < '
               '6000.0,-2.88526e+32*T^(-9.0) - 50.0*T*log(T) + 344.18*T - 34869.344,0)) + '
               'BCC_FE*BCC_VA*if(T >= 298.15 & T < 1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T '
               '- 0.00439752*T^2.0 - 5.89269e-8*T^3.0 + 1225.7,if(T >= 1811.0 & T < '
               '6000.0,2.2960305e+31*T^(-9.0) - 46.0*T*log(T) + 299.31255*T - 25383.581,0)))/(BCC_CR '
               '+ BCC_FE) + 1.0*(BCC_CR*BCC_FE*BCC_VA*(500.0 - 1.5*T)*(BCC_CR - BCC_FE) + '
               'BCC_CR*BCC_FE*BCC_VA*(24600.0 - 14.98*T) + BCC_CR*BCC_FE*BCC_VA*(9.15*T - '
               '14000.0)*(BCC_CR - BCC_FE)^2)/(BCC_CR + BCC_FE); G/100000'
    args = 'BCC_CR'
    constant_names = 'BCC_VA T'
    constant_expressions = '1 1000'
  []

  [F_SIGMA]
    type = DerivativeParsedMaterial
    f_name = F_SIGMA
    outputs = exodus
    output_properties = F_SIGMA
    function = 'SIGMA_0FE := 1-SIGMA_0CR; SIGMA_1FE := 1-SIGMA_1CR; SIGMA_2FE := 1-SIGMA_2CR; '
               'G := 8.3145*T*(10.0*if(SIGMA_0CR > 1.0e-15,SIGMA_0CR*log(SIGMA_0CR),0) + '
               '10.0*if(SIGMA_0FE > 1.0e-15,SIGMA_0FE*log(SIGMA_0FE),0) + 4.0*if(SIGMA_1CR > '
               '1.0e-15,SIGMA_1CR*log(SIGMA_1CR),0) + 4.0*if(SIGMA_1FE > '
               '1.0e-15,SIGMA_1FE*log(SIGMA_1FE),0) + 16.0*if(SIGMA_2CR > '
               '1.0e-15,SIGMA_2CR*log(SIGMA_2CR),0) + 16.0*if(SIGMA_2FE > '
               '1.0e-15,SIGMA_2FE*log(SIGMA_2FE),0))/(10.0*SIGMA_0CR + 10.0*SIGMA_0FE + '
               '4.0*SIGMA_1CR + 4.0*SIGMA_1FE + 16.0*SIGMA_2CR + 16.0*SIGMA_2FE) + '
               '(SIGMA_0FE*SIGMA_1CR*SIGMA_2CR*SIGMA_2FE*(-70.0*T - 170400.0) + '
               'SIGMA_0FE*SIGMA_1FE*SIGMA_2CR*SIGMA_2FE*(-10.0*T - 330839.0))/(10.0*SIGMA_0CR + '
               '10.0*SIGMA_0FE + 4.0*SIGMA_1CR + 4.0*SIGMA_1FE + 16.0*SIGMA_2CR + 16.0*SIGMA_2FE) + '
               '(SIGMA_0CR*SIGMA_1CR*SIGMA_2CR*(30.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - '
               '26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,if(T >= '
               '2180.0 & T < 6000.0,-2.88526e+32*T^(-9.0) - 50.0*T*log(T) + 344.18*T - 34869.344,0)) '
               '+ 132000.0) + SIGMA_0CR*SIGMA_1CR*SIGMA_2FE*(-110.0*T + 16.0*if(T >= 298.15 & T < '
               '1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - '
               '5.89269e-8*T^3.0 + 1225.7,if(T >= 1811.0 & T < 6000.0,2.2960305e+31*T^(-9.0) - '
               '46.0*T*log(T) + 299.31255*T - 25383.581,0)) + 14.0*if(T >= 298.15 & T < '
               '2180.0,139250.0*1/T - 26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - '
               '1.47721e-6*T^3.0 - 8856.94,if(T >= 2180.0 & T < 6000.0,-2.88526e+32*T^(-9.0) - '
               '50.0*T*log(T) + 344.18*T - 34869.344,0)) + 123500.0) + '
               'SIGMA_0CR*SIGMA_1FE*SIGMA_2CR*(4.0*if(T >= 298.15 & T < 1811.0,77358.5*1/T - '
               '23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - 5.89269e-8*T^3.0 + 1225.7,if(T >= '
               '1811.0 & T < 6000.0,2.2960305e+31*T^(-9.0) - 46.0*T*log(T) + 299.31255*T - '
               '25383.581,0)) + 26.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - 26.908*T*log(T) + '
               '157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,if(T >= 2180.0 & T < '
               '6000.0,-2.88526e+32*T^(-9.0) - 50.0*T*log(T) + 344.18*T - 34869.344,0)) + 140486.0) '
               '+ SIGMA_0CR*SIGMA_1FE*SIGMA_2FE*(20.0*if(T >= 298.15 & T < 1811.0,77358.5*1/T - '
               '23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - 5.89269e-8*T^3.0 + 1225.7,if(T >= '
               '1811.0 & T < 6000.0,2.2960305e+31*T^(-9.0) - 46.0*T*log(T) + 299.31255*T - '
               '25383.581,0)) + 10.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - 26.908*T*log(T) + '
               '157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,if(T >= 2180.0 & T < '
               '6000.0,-2.88526e+32*T^(-9.0) - 50.0*T*log(T) + 344.18*T - 34869.344,0)) + 148800.0) '
               '+ SIGMA_0FE*SIGMA_1CR*SIGMA_2CR*(10.0*if(T >= 298.15 & T < 1811.0,77358.5*1/T - '
               '23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - 5.89269e-8*T^3.0 + 1225.7,if(T >= '
               '1811.0 & T < 6000.0,2.2960305e+31*T^(-9.0) - 46.0*T*log(T) + 299.31255*T - '
               '25383.581,0)) + 20.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - 26.908*T*log(T) + '
               '157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,if(T >= 2180.0 & T < '
               '6000.0,-2.88526e+32*T^(-9.0) - 50.0*T*log(T) + 344.18*T - 34869.344,0)) + 56200.0) + '
               'SIGMA_0FE*SIGMA_1CR*SIGMA_2FE*(26.0*if(T >= 298.15 & T < 1811.0,77358.5*1/T - '
               '23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - 5.89269e-8*T^3.0 + 1225.7,if(T >= '
               '1811.0 & T < 6000.0,2.2960305e+31*T^(-9.0) - 46.0*T*log(T) + 299.31255*T - '
               '25383.581,0)) + 4.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - 26.908*T*log(T) + '
               '157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,if(T >= 2180.0 & T < '
               '6000.0,-2.88526e+32*T^(-9.0) - 50.0*T*log(T) + 344.18*T - 34869.344,0)) + 152700.0) '
               '+ SIGMA_0FE*SIGMA_1FE*SIGMA_2CR*(14.0*if(T >= 298.15 & T < 1811.0,77358.5*1/T - '
               '23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - 5.89269e-8*T^3.0 + 1225.7,if(T >= '
               '1811.0 & T < 6000.0,2.2960305e+31*T^(-9.0) - 46.0*T*log(T) + 299.31255*T - '
               '25383.581,0)) + 16.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - 26.908*T*log(T) + '
               '157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,if(T >= 2180.0 & T < '
               '6000.0,-2.88526e+32*T^(-9.0) - 50.0*T*log(T) + 344.18*T - 34869.344,0)) + 46200.0) + '
               'SIGMA_0FE*SIGMA_1FE*SIGMA_2FE*(30.0*if(T >= 298.15 & T < 1811.0,77358.5*1/T - '
               '23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - 5.89269e-8*T^3.0 + 1225.7,if(T >= '
               '1811.0 & T < 6000.0,2.2960305e+31*T^(-9.0) - 46.0*T*log(T) + 299.31255*T - '
               '25383.581,0)) + 173333.0))/(10.0*SIGMA_0CR + 10.0*SIGMA_0FE + 4.0*SIGMA_1CR + '
               '4.0*SIGMA_1FE + 16.0*SIGMA_2CR + 16.0*SIGMA_2FE); G/100000'
    args = 'SIGMA_0CR SIGMA_1CR SIGMA_2CR'
    constant_names = 'T'
    constant_expressions = '1000'
  []

  # h(eta)
  [h_eta]
    type = SwitchingFunctionMaterial
    function_name = h1
    h_order = HIGH
    eta = eta1
  []

  # g(eta)
  [g_eta]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta1
  []

  # constant properties
  [constants]
    type = GenericConstantMaterial
    prop_names  = 'D   L   kappa'
    prop_values = '10  1   1  '
  []

  [h2]
    type = DerivativeParsedMaterial
    material_property_names = 'h1(eta1)'
    args = eta1
    function = 1-h1
    f_name = h2
  []

  # Coefficients for diffusion equation
  [Dh1]
    type = DerivativeParsedMaterial
    material_property_names = 'D h1(eta1)'
    function = D*h1
    f_name = Dh1
    args = eta1
  []
  [Dh2a]
    type = DerivativeParsedMaterial
    material_property_names = 'D h2(eta1)'
    function = D*h2*10/30
    f_name = Dh2a
    args = eta1
  []
  [Dh2b]
    type = DerivativeParsedMaterial
    material_property_names = 'D h2(eta1)'
    function = D*h2*4/30
    f_name = Dh2b
    args = eta1
  []
  [Dh2c]
    type = DerivativeParsedMaterial
    material_property_names = 'D h2(eta1)'
    function = D*h2*16/30
    f_name = Dh2c
    args = eta1
  []
[]

[Kernels]
  #Kernels for diffusion equation
  [diff_time]
    type = TimeDerivative
    variable = cCr
  []
  [diff_c1]
    type = MatDiffusion
    variable = cCr
    diffusivity = Dh1
    v = BCC_CR
    args = eta1
  []
  [diff_c2a]
    type = MatDiffusion
    variable = cCr
    diffusivity = Dh2a
    v = SIGMA_0CR
    args = eta1
  []
  [diff_c2b]
    type = MatDiffusion
    variable = cCr
    diffusivity = Dh2b
    v = SIGMA_1CR
    args = eta1
  []
  [diff_c2c]
    type = MatDiffusion
    variable = cCr
    diffusivity = Dh2c
    v = SIGMA_2CR
    args = eta1
  []

  # enforce pointwise equality of chemical potentials
  [ChemPotSolute]
    # The BCC phase has only one sublattice
    # we tie it to the first sublattice with site fraction 3/10 in the sigma phase
    type = KKSPhaseChemicalPotential
    variable = BCC_CR
    cb = SIGMA_0CR
    kb = '${fparse 3/10}'
    fa_name = F_BCC_A2
    fb_name = F_SIGMA

  []
  [chempot2a2b]
    # This kernel ties the first two sublattices in the sigma phase together
    type = SLKKSChemicalPotential
    variable = SIGMA_0CR
    a = 10
    cs = SIGMA_1CR
    as = 4
    F = F_SIGMA
  []
  [chempot2b2c]
    # This kernel ties the remaining two sublattices in the sigma phase together
    type = SLKKSChemicalPotential
    variable = SIGMA_1CR
    a = 4
    cs = SIGMA_2CR
    as = 16
    F = F_SIGMA
  []

  [phaseconcentration]
    # This kernel ties the sum of the sublattice concentrations to the global concentration cCr
    type = SLKKSMultiPhaseConcentration
    variable = SIGMA_2CR
    c = cCr
    ns = '1      3'
    as = '1      10        4         16'
    cs = 'BCC_CR SIGMA_0CR SIGMA_1CR SIGMA_2CR'
    h_names = 'h1   h2'
    eta = 'eta1'
  []

  # Kernels for Allen-Cahn equation for eta1
  [deta1dt]
    type = TimeDerivative
    variable = eta1
  []
  [ACBulkF1]
    type = KKSMultiACBulkF
    variable = eta1
    Fj_names = 'F_BCC_A2 F_SIGMA'
    hj_names = 'h1    h2'
    gi_name = g1
    eta_i = eta1
    wi = 1.0
    args = 'BCC_CR SIGMA_0CR SIGMA_1CR SIGMA_2CR'
  []
  [ACBulkC1]
    type = SLKKSMultiACBulkC
    variable = eta1
    F = F_BCC_A2
    c = BCC_CR
    ns = '1      3'
    as = '1      10        4         16'
    cs = 'BCC_CR SIGMA_0CR SIGMA_1CR SIGMA_2CR'
    h_names = 'h1   h2'
    eta = 'eta1'
  []
  [ACInterface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa
  []
[]

[AuxKernels]
  [GlobalFreeEnergy]
    type = KKSGlobalFreeEnergy
    variable = Fglobal
    fa_name = F_BCC_A2
    fb_name = F_SIGMA
    h_name = h1
    interfacial_vars = eta1
    kappa_names = kappa
    w = 2.0
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'asm      lu          nonzero                    30'

  l_max_its = 100
  nl_max_its = 20
  nl_abs_tol = 1e-10

  end_time = 10000

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 12
    iteration_window = 2
    growth_factor = 1.5
    cutback_factor = 0.7
    dt = 1e-5
  []
[]

[VectorPostprocessors]
  [c]
    type = LineValueSampler
    start_point = '-25 0 0'
    end_point = '25 0 0'
    variable = cCr
    num_points = 151
    sort_by = id
    execute_on = timestep_end
  []
  [eta1]
    type = LineValueSampler
    start_point = '-25 0 0'
    end_point = '25 0 0'
    variable = eta1
    num_points = 151
    sort_by = id
    execute_on = timestep_end
  []
[]

[Postprocessors]
  [F]
    type = ElementIntegralVariablePostprocessor
    variable = Fglobal
  []
[]

[Debug]
  # show_var_residual_norms = true
[]

[Outputs]
  interval = 20
  exodus = true
  print_linear_residuals = false
  csv = true
[]
