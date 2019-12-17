# ADCHSplitChemicalPotential

!syntax description /Kernels/ADCHSplitChemicalPotential

Uses automatic differentiation to solve for chemical potential in a weak sense (mu-mu_prop=0).  Can
be coupled to Cahn-Hilliard equation to solve species diffusion.  Allows spatial derivative of
chemical potential when coupled to material state such as stress, etc.  Can be used to model
species diffusion mediated creep.  Typically used with
[ADCHSplitConcentration](/ADCHSplitConcentration.md) and can combined with
[ADCHSoretMobility](/ADCHSoretMobility.md) to incorporate the effects of thermodiffusion.  

!syntax parameters /Kernels/ADCHSplitChemicalPotential

!syntax inputs /Kernels/ADCHSplitChemicalPotential

!syntax children /Kernels/ADCHSplitChemicalPotential
