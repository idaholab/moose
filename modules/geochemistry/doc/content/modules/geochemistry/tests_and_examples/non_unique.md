# Examples of models that exhibit nonunique solutions

This page follows Chapter 12 of [!cite](bethke_2007).

## Boehmite equilibrium

Assume:

- temperature is $200^{\circ}$C;
- the mineral Boehmite is used in place of H$^{+}$ in the component basis;
- there is 1$\,$cm$^{3}$ of free Boehmite;
- supersaturation of all minerals is ignored;
- the concentrations of the other components are given in [table:boehmite].

!table id=table:boehmite caption=Bulk concentration in the Boehmite example
| Species | Concentration (mmol/kg(solvent water)) |
| --- | --- |
| K$^{+}$ | 100 |
| Cl$^{-}$ | 100 |
| SiO$_{2}$(aq) | 3 |
| HCO$_{3}^{-}$ | 0.06 |

### Case 1

Assume the bulk concentration of Al$^{3+}$ is $10^{-5}\,$mol/kg(solvent water).

### Case 2

Use Al(OH)$_{4}^{-}$ instead of Al$^{3+}$ in the component basis, with a concentration of $10^{-5}\,$mol/kg(solvent water).

### Results

[!cite](bethke_2007) predicts the composition at equilibrium to be as shown in [table:b_results].  MOOSE produces the result ????

!table id=table:b_results caption=Equilibrium composition in the Boehmite example
| Quantity | Case 1 | Case 2 |
| --- | --- | --- |
| pH | 3.1 | 6.3 |
| total Al$^{3+}$ ($\mu$molal) | 10 | 10 |
| free AlOH$^{2+}$ ($\mu$molal) | 4.3 | $1.5\times 10^{-6}$ | 
| free Al(OH)$_{2}^{+}$ ($\mu$molal) | 3.0 | $1.8\times 10^{-9}$ | 
| free Al(OH)$_{4}^{-}$ ($\mu$molal) | $3.5\times 10^{-9}$ | $9.2\times 10^{-6}$ | 


## Pyrite example

Assume:

- temperature is $100^{\circ}$C;
- the mineral pyrite is used in place of O$_{2}$(aq) in the component basis;
- there is 1$\,$cm$^{3}$ of free pyrite;
- the pH is 4
- supersaturation of all minerals is ignored;
- the concentrations of the other components are given in [table:pyrite]

!table id=table:pyrite caption=Bulk concentration in the Pyrite example
| Species | Concentration (mol/kg(solvent water)) |
| --- | --- |
| Na$^{+}$ | 1 |
| Cl$^{-}$ | 1 |
| Fe$^{2+}$ | $10^{-2}$ |

### Case 1

Assume the bulk concentration of SO$_{4}^{2-}$ is 10$\,$mmolal

### Case 2

Use H$_{2}$S(aq) instead of SO$_{4}^{2-}$ in the component basis and its bulk concentration is 10$\,$mmolal.

### Results

[!cite](bethke_2007) predicts the composition at equilibrium to be as shown in [table:pyrite_results].  MOOSE produces the result ????

!table id=table:pyrite_results caption=Equilibrium composition in the Pyrite example
| Quantity | Case 1 | Case 2 |
| --- | --- | --- |
| pH | 4 | 4 |
| $\log_{10}f_{\mathrm{O}_{2}}$ | -50 | -67 |
| total Fe$^{2+}$ (molal) | 0.01 | 0.01 |
| total S (molal) | 0.01 | 0.01 |
| $\sum$SO$_{4}^{2-}$ (molal) | 0.010 | $5.0\times 10^{-34}$ |
| $\sum$H$_{2}$S(aq) (molal) | $3.7\times 10^{-7}$ | 0.010 | 


!bibtex bibliography