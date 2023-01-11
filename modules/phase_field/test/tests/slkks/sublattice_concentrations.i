#
# SLKKS two phase example for the BCC and SIGMA phases. The sigma phase contains
# multiple sublattices. Free energy from
# Jacob, Aurelie, Erwin Povoden-Karadeniz, and Ernst Kozeschnik. "Revised thermodynamic
# description of the Fe-Cr system based on an improved sublattice model of the sigma phase."
# Calphad 60 (2018): 16-28.
#
# In this simulation we solve only for the sublattice concentrations of the sigma phase
# (and consequently for the free energy of the sigma phase as function of total concentration.)
# The Cr concentration is prescribed as linear gradient. This permits us to plot
# the free energies of the BCC and sigma phases.
#

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
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

[Kernels]
  [chempot2a2b]
    # This kernel ties the first two sublattices in the sigma phase together
    type = SLKKSChemicalPotential
    variable = SIGMA_0CR
    a = 10
    cs = SIGMA_1CR
    as = 4
    F = F_SIGMA
    coupled_variables = SIGMA_2CR
  []
  [chempot2b2c]
    # This kernel ties the remaining two sublattices in the sigma phase together
    type = SLKKSChemicalPotential
    variable = SIGMA_1CR
    a = 4
    cs = SIGMA_2CR
    as = 16
    F = F_SIGMA
    coupled_variables = SIGMA_0CR
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
    outputs = exodus
    expression = 'SIGMA_0FE := 1-SIGMA_0CR;
                SIGMA_1FE := 1-SIGMA_1CR;SIGMA_2FE := '
               '1-SIGMA_2CR; 8.3145*T*(10.0*if(SIGMA_0CR > 1.0e-15,SIGMA_0CR*log(SIGMA_0CR),0) + '
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
               '26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,0) + '
               '132000.0) + SIGMA_0CR*SIGMA_1CR*SIGMA_2FE*(-110.0*T + 16.0*if(T >= 298.15 & T < '
               '1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - '
               '5.89269e-8*T^3.0 + 1225.7,0) + 14.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - '
               '26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,0) + '
               '123500.0) + SIGMA_0CR*SIGMA_1FE*SIGMA_2CR*(4.0*if(T >= 298.15 & T < '
               '1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - '
               '5.89269e-8*T^3.0 + 1225.7,0) + 26.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - '
               '26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,0) + '
               '140486.0) + SIGMA_0CR*SIGMA_1FE*SIGMA_2FE*(20.0*if(T >= 298.15 & T < '
               '1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - '
               '5.89269e-8*T^3.0 + 1225.7,0) + 10.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - '
               '26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,0) + '
               '148800.0) + SIGMA_0FE*SIGMA_1CR*SIGMA_2CR*(10.0*if(T >= 298.15 & T < '
               '1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - '
               '5.89269e-8*T^3.0 + 1225.7,0) + 20.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - '
               '26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,0) + '
               '56200.0) + SIGMA_0FE*SIGMA_1CR*SIGMA_2FE*(26.0*if(T >= 298.15 & T < '
               '1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - '
               '5.89269e-8*T^3.0 + 1225.7,0) + 4.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - '
               '26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,0) + '
               '152700.0) + SIGMA_0FE*SIGMA_1FE*SIGMA_2CR*(14.0*if(T >= 298.15 & T < '
               '1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - '
               '5.89269e-8*T^3.0 + 1225.7,0) + 16.0*if(T >= 298.15 & T < 2180.0,139250.0*1/T - '
               '26.908*T*log(T) + 157.48*T + 0.00189435*T^2.0 - 1.47721e-6*T^3.0 - 8856.94,0) + '
               '46200.0) + SIGMA_0FE*SIGMA_1FE*SIGMA_2FE*(30.0*if(T >= 298.15 & T < '
               '1811.0,77358.5*1/T - 23.5143*T*log(T) + 124.134*T - 0.00439752*T^2.0 - '
               '5.89269e-8*T^3.0 + 1225.7,0) + 173333.0))/(10.0*SIGMA_0CR + 10.0*SIGMA_0FE + '
               '4.0*SIGMA_1CR + 4.0*SIGMA_1FE + 16.0*SIGMA_2CR + 16.0*SIGMA_2FE)'
    coupled_variables = 'SIGMA_0CR SIGMA_1CR SIGMA_2CR'
    constant_names = 'T'
    constant_expressions = '1000'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
