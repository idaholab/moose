#
# SLKKS two phase example for the BCC and SIGMA phases. The sigma phase contains
# multiple sublattices. Free energy from
# Jacob, Aurelie, Erwin Povoden-Karadeniz, and Ernst Kozeschnik. "Revised thermodynamic
# description of the Fe-Cr system based on an improved sublattice model of the sigma phase."
# Calphad 60 (2018): 16-28.
#
# In this simulation we solve only for the sublattice concentrations of the sigma phase
# (and consequently for the free energy of the sigma phase as function of total concentration.)
# The Cr concentration is prescribed as a linear gradient. This permits us to plot
# the free energies of the BCC and sigma phases.
#

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1000
    ny = 1
    xmin = 0.01
    xmax = 0.99
    ymax = 0.1
  []
[]

[AuxVariables]
  [cCr]
    [InitialCondition]
      type = FunctionIC
      function = x
    []
  []
  [Fb]
    order = FIRST
    family = MONOMIAL
  []
  [Fs]
    order = FIRST
    family = MONOMIAL
  []
  [dFs]
    order = CONSTANT
    family = MONOMIAL
  []
  [dFs0]
    order = CONSTANT
    family = MONOMIAL
  []
  [dFs1]
    order = CONSTANT
    family = MONOMIAL
  []
  [dFs2]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Variables]
  [SIGMA_0CR]
    [InitialCondition]
      type = FunctionIC
      function = x
    []
  []
  [SIGMA_1CR]
    [InitialCondition]
      type = FunctionIC
      function = x
    []
  []
  [SIGMA_2CR]
    [InitialCondition]
      type = FunctionIC
      function = x
    []
  []
[]

[AuxKernels]
  [Fb]
    type = MaterialRealAux
    variable = Fb
    property = F_BCC_A2
  []
  [Fs]
    type = MaterialRealAux
    variable = Fs
    property = F_SIGMA
  []
  [dFs0]
    type = MaterialRealAux
    variable = dFs0
    property = dF_SIGMA/dSIGMA_0CR
  []
  [dFs1]
    type = MaterialRealAux
    variable = dFs1
    property = dF_SIGMA/dSIGMA_1CR
  []
  [dFs2]
    type = MaterialRealAux
    variable = dFs1
    property = dF_SIGMA/dSIGMA_2CR
  []
  [dFs]
    type = VariableGradientComponent
    variable = dFs
    gradient_variable = Fs
    component = x
  []
[]

[Kernels]
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
  [sum]
    type = SLKKSSum
    variable = SIGMA_2CR
    a = 16
    cs = 'SIGMA_0CR SIGMA_1CR'
    as = '10        4'
    sum = cCr
  []
[]

[Materials]
  # CALPHAD free energy of the FeCr sigma phase
  [F_SIGMA]
    type = DerivativeParsedMaterial
    property_name = F_SIGMA
    expression = 'SIGMA_0FE := 1-SIGMA_0CR;
                SIGMA_1FE := 1-SIGMA_1CR;
                '
               'SIGMA_2FE := 1-SIGMA_2CR; 8.3145*T*(10.0*if(SIGMA_0CR > '
               '1.0e-15,SIGMA_0CR*log(SIGMA_0CR),0) + 10.0*if(SIGMA_0FE > '
               '1.0e-15,SIGMA_0FE*log(SIGMA_0FE),0) + 4.0*if(SIGMA_1CR > '
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
               '4.0*SIGMA_1FE + 16.0*SIGMA_2CR + 16.0*SIGMA_2FE)'
    coupled_variables = 'SIGMA_0CR SIGMA_1CR SIGMA_2CR'
    constant_names = 'T'
    constant_expressions = '1000'
  []

  # single sublattice BCC phase (no sublattice concentration solve necessary)
  [F_BCC_A2]
    type = DerivativeParsedMaterial
    property_name = F_BCC_A2
    expression = 'BCC_CR:=cCr; BCC_FE:=1-BCC_CR; 8.3145*T*(1.0*if(BCC_CR > '
               '1.0e-15,BCC_CR*log(BCC_CR),0) + 1.0*if(BCC_FE > 1.0e-15,BCC_FE*log(BCC_FE),0) + '
               '3.0*if(BCC_VA > 1.0e-15,BCC_VA*log(BCC_VA),0))/(BCC_CR + BCC_FE) + 8.3145*T*if(T < '
               '548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - 932.5*BCC_CR*BCC_FE*BCC_VA + '
               '311.5*BCC_CR*BCC_VA - '
               '1043.0*BCC_FE*BCC_VA,-8.13674105561218e-49*T^15/(0.525599232981783*BCC_CR*BCC_FE*BCC_'
               'VA*(BCC_CR - BCC_FE) - 0.894055608820709*BCC_CR*BCC_FE*BCC_VA + '
               '0.298657718120805*BCC_CR*BCC_VA - BCC_FE*BCC_VA + 9.58772770853308e-13)^15 - '
               '4.65558036243985e-30*T^9/(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^9 - '
               '1.3485349181899e-10*T^3/(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^3 + 1 - '
               '0.905299382744392*(548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - '
               '932.5*BCC_CR*BCC_FE*BCC_VA + 311.5*BCC_CR*BCC_VA - 1043.0*BCC_FE*BCC_VA + '
               '1.0e-9)/T,if(T < -548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '932.5*BCC_CR*BCC_FE*BCC_VA - 311.5*BCC_CR*BCC_VA + '
               '1043.0*BCC_FE*BCC_VA,-8.13674105561218e-49*T^15/(-0.525599232981783*BCC_CR*BCC_FE*BCC'
               '_VA*(BCC_CR - BCC_FE) + 0.894055608820709*BCC_CR*BCC_FE*BCC_VA - '
               '0.298657718120805*BCC_CR*BCC_VA + BCC_FE*BCC_VA + 9.58772770853308e-13)^15 - '
               '4.65558036243985e-30*T^9/(-0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) '
               '+ 0.894055608820709*BCC_CR*BCC_FE*BCC_VA - 0.298657718120805*BCC_CR*BCC_VA + '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^9 - '
               '1.3485349181899e-10*T^3/(-0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA - 0.298657718120805*BCC_CR*BCC_VA + '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^3 + 1 - '
               '0.905299382744392*(-548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '932.5*BCC_CR*BCC_FE*BCC_VA - 311.5*BCC_CR*BCC_VA + 1043.0*BCC_FE*BCC_VA + '
               '1.0e-9)/T,if(T > -548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '932.5*BCC_CR*BCC_FE*BCC_VA - 311.5*BCC_CR*BCC_VA + 1043.0*BCC_FE*BCC_VA & '
               '548.2*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - 932.5*BCC_CR*BCC_FE*BCC_VA + '
               '311.5*BCC_CR*BCC_VA - 1043.0*BCC_FE*BCC_VA < '
               '0,-79209031311018.7*(-0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) + '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA - 0.298657718120805*BCC_CR*BCC_VA + '
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
               '0,-79209031311018.7*(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^5/T^5 - '
               '3.83095660520737e+42*(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^15/T^15 - '
               '1.22565886734485e+72*(0.525599232981783*BCC_CR*BCC_FE*BCC_VA*(BCC_CR - BCC_FE) - '
               '0.894055608820709*BCC_CR*BCC_FE*BCC_VA + 0.298657718120805*BCC_CR*BCC_VA - '
               'BCC_FE*BCC_VA + 9.58772770853308e-13)^25/T^25,0))))*log((2.15*BCC_CR*BCC_FE*BCC_VA - '
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
               '14000.0)*(BCC_CR - BCC_FE)^2)/(BCC_CR + BCC_FE)'

    coupled_variables = 'cCr'
    constant_names = 'BCC_VA T'
    constant_expressions = '1 1000'
  []
[]

[VectorPostprocessors]
  [var]
    type = LineValueSampler
    variable = 'SIGMA_0CR SIGMA_1CR SIGMA_2CR cCr Fb Fs dFs dFs0 dFs1 dFs2'
    start_point = '0.01 0 0'
    end_point = '0.99 0 0'
    sort_by = x
    num_points = 1000
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]
