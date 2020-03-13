# Dumping minerals and then adding chemicals

Chapter 13.3 of [!cite](bethke_2007) describes a "dump" process, whereby a mineral is added to the initial solution, equilibrium sought, then any undissolved mineral is removed.  In the code, this is achieved by:

- subtracting $n_{k}$ from $M_{k}$;
- then setting $n_{k}=0$;
- setting the mole numbers of any surface components to zero, $M_{p}=0$, as well as the molalities of unoccupied surface sites, $m_{p}=0$ and adsorbed species, $m_{q}=0$;
- finally, swapping an appropriate aqueous species $A_{j}$ into the basis in place of $A_{k}$.

After this process, other chemicals can be added to this system, and/or the temperature varied, and/or the pH varied, etc.

Section 15.2 of [!cite](bethke_2007) provides an example of this "dump" process.  Assume:

- the mineral calcite is used in the basis instead of HCO$_{3}^{-}$, and that the free volume of calcite is 10$\,$cm$^{3}$;
- the pH is initially 8;
- the concentration of Na$^{+}$ is 100$\,$mmolal;
- the concentration of Ca$^{2+}$ is 10$\,$mmolal;
- charge balance is enforced on Cl$^{-}$.

### Case 1

The system is brought to equilibrium and then the calcite is "dumped".  After this, 100$\,$mmol of HCl is added to the system.

### Case 2

The system is brought to equilibrium and but the calcite is not "dumped".  After this, 100$\,$mmol of HCl is added to the system.

### Results

[!cite](bethke_2007) presents the results in Figures 15.6 and 15.7.

MOOSE produces the result ????




!bibtex bibliography