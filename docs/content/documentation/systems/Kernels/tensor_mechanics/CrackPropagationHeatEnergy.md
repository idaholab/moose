# CrackPropagationHeatEnergy
Provides a heat source from crack propagation:
- (dPsi/dc) * (dc/dt)
Psi is the free energy of the phase-field fracture model
defined as Psi = (1 - c)^2 * G0_pos + G0_neg
c is the order parameter for damage, continuous between 0 and 1
0 represents no damage, 1 represents fully cracked
G0_pos and G0_neg are the positive and negative components
of the specific strain energies
- (dPsi/dc) * (dc/dt) = 2 * (1 - c) * G0_pos * (dc/dt)
C. Miehe, L.M. Schanzel, H. Ulmer, Comput. Methods Appl. Mech. Engrg. 294 (2015) 449 - 485
P. Chakraborty, Y. Zhang, M.R. Tonks, Multi-scale modeling of microstructure dependent
inter-granular fracture in UO2 using a phase-field based method
Idaho National Laboratory technical report

!syntax description /Kernels/CrackPropagationHeatEnergy

!syntax parameters /Kernels/CrackPropagationHeatEnergy

!syntax inputs /Kernels/CrackPropagationHeatEnergy

!syntax children /Kernels/CrackPropagationHeatEnergy
