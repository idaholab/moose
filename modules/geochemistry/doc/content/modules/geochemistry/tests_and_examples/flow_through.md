# Flow-through reactions that remove minerals

Chapter 13.3 of [!cite](bethke_2007) describes a "flow-through" process, whereby a mineral is removed from the system at each stage in a reaction process (such as progressively adding chemicals, changing the temperature, changing the pH, etc).  In the code, this is achieved by the following process, after each stage's equilibrium configuration is computed:

- subtracting $n_{k}$ from $M_{k}$;
- then setting $n_{k}$ to a tiny number (but not zero, otherwise it might be swapped out of the basis);
- setting the mole numbers of any surface components to zero, $M_{p}=0$, as well as the molalities of unoccupied surface sites, $m_{p}=0$ and adsorbed species, $m_{q}=0$.

TODO: Section 24.3 of Bethke

!bibtex bibliography