# Sorption onto ferric hydroxide

This example closely follows Section 10.4 of [!cite](bethke_2007).

We explore the sorption of mercury, lead and sulfate onto hydrous ferric oxide at pH 4 and 8.

## Definition of the sorption via surface complexation

It is assumed that there are two sorbing sites:
\begin{equation}
A_{p}=\left(\ \mathrm{>(s)FeOH},\  \mathrm{>(w)FeOH}\,\right) \ .
\end{equation}
[!cite](bethke_2007) uses the convention that a ">" indicates something to do with sorption.  The sorbing reactions, written in the [documentation on equilibrium reactions](equilibrium.md) as
\begin{equation}
A_{q} \rightleftharpoons \nu_{wq}A_{w} + \sum_{i}\nu_{iq}A_{i} + \sum_{k}\nu_{kq}A_{k} + \sum_{m}\nu_{mq}A_{m} + \nu_{pq}A_{p} \ ,
\end{equation}
are, specifically:
\begin{equation}
\begin{aligned}
\mathrm{>(w)FeOH}_{2}^{+} & \rightleftharpoons \mathrm{>(w)FeOH} + \mathrm{H}^{+} & \log_{10}K=-7.29\ , \\
\mathrm{>(w)FeO}^{-} & \rightleftharpoons \mathrm{>(w)FeOH} - \mathrm{H}^{+} & \log_{10}K=8.93\ , \\
\mathrm{>(w)FeOHg}^{+} & \rightleftharpoons \mathrm{>(w)FeOH} - \mathrm{H}^{+} + \mathrm{Hg}^{2+} & \log_{10}K=-6.45\ , \\
\mathrm{>(w)FeOPb}^{+} & \rightleftharpoons \mathrm{>(w)FeOH} - \mathrm{H}^{+} + \mathrm{Pb}^{2+} & \log_{10}K=-0.3\ , \\
\mathrm{>(w)FeSO}_{4}^{-} & \rightleftharpoons \mathrm{>(w)FeOH} + \mathrm{H}^{+} + \mathrm{SO}_{4}^{2-}  - \mathrm{H}_{2}\mathrm{O} & \log_{10}K=-7.78\ , \\
\mathrm{>(w)FeOHSO}_{4}^{2-} & \rightleftharpoons \mathrm{>(w)FeOH} + \mathrm{SO}_{4}^{2-} & \log_{10}K=-0.79\ , \\
\mathrm{>(s)FeOH}_{2}^{+} & \rightleftharpoons \mathrm{>(s)FeOH} + \mathrm{H}^{+} & \log_{10}K=-7.29\ , \\
\mathrm{>(s)FeO}^{-} & \rightleftharpoons \mathrm{>(s)FeOH} - \mathrm{H}^{+} & \log_{10}K=8.93\ , \\
\mathrm{>(s)FeOHg}^{+} & \rightleftharpoons \mathrm{>(s)FeOH} - \mathrm{H}^{+} + \mathrm{Hg}^{2+} & \log_{10}K=-7.76\ , \\
\mathrm{>(s)FeOPb}^{+} & \rightleftharpoons \mathrm{>(s)FeOH} - \mathrm{H}^{+} + \mathrm{Pb}^{2+} & \log_{10}K=-4.65\ , \\
\end{aligned}
\end{equation}
The sorption occurs on the ferric hydroxide mineral, Fe(OH)$_{3}$, called Fe(OH)3(ppd) in the database.

In order to complete the description of surface complexation, the surface potential $\Psi$ must be specified.  This requires the specific surface area, which is assumed to be
\begin{equation}
A_{s} = 600\,\mathrm{m}^{2}/\mathrm{g(mineral)} \ ,
\end{equation}
We assume that:

- For each mol of Fe(OH)3(ppd), there is 0.005$\,$mol of >(s)FeOH
- For each mol of Fe(OH)3(ppd), there is 0.2$\,$mol of >(w)FeOH

All of the above information is contained in the MOOSE test database `ferric_hydroxide_sorption.json`.  It is read into the MOOSE input file using a [GeochemicalModelDefinition](GeochemicalModelDefinition.md) UserObject

!listing modules/geochemistry/test/tests/sorption_and_surface_complexation/ferric_hydroxide.i block=UserObjects

## Chemical composition, mineral quantities and sorbing moles

The chemical composition of the water is shown in [table:analysis].  In addition:

- charge balance is performed on Cl$^{-}$;
- two models are run: one with pH=4, and the other with pH=8.

!table id=table:analysis caption=Element composition of water in the sorption example
| Species | Concentration (mmol.kg$^{-1}$) |
| --- | --- |
| Na$^{+}$ | 10 | 
| Cl$^{-}$ | 10 |
| Hg$^{2+}$ | 0.1 |
| Pb$^{2+}$ | 0.1 |
| SO$_{4}^{2-}$ | 0.2 |

There is 1 free gram of Fe(OH)3(ppd), which in `geochemistry` language means there is $9.357\times 10^{-3}$ free mols of this mineral.  Remember "free" moles means a quantity that is "floating around in the aqueous solution".  This is in contrast to a "bulk composition" which consists of the free amount as well as an amount that forms equilibrium (secondary) species.  This means there is

- $0.005 \times 9.357\times 10^{-3} = 4.6786\times 10^{-5}\,$mol of >(s)FeOH
- $0.2 \times 9.357\times 10^{-3} = 1.87145\times 10^{-3}\,$mol of >(w)FeOH

Note that these are *bulk* composition values as some of these sites will be free and some will be occupied by sorbed species.

!alert note
The bulk mole compositions of the sites >(s)FeOH and >(w)FeOH must be specified in the geochemistry module.  This is different from Geochemists Workbench, which works out the bulk compositions internally.

The [TimeIndependentReactionSolver](AddTimeIndependentReactionSolverAction.md) defines the composition, including the pH, the free mole number of the mineral, and the bulk composition of the sorbing sites:

!listing modules/geochemistry/test/tests/sorption_and_surface_complexation/ferric_hydroxide.i block=TimeIndependentReactionSolver

## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/sorption_and_surface_complexation/ferric_hydroxide.rea

## Results

[!cite](bethke_2007) computes the results that are shown in [table:surface], [table:weak_conc], [table:strong_conc] and [table:perc_sorbed].  Both the `geochemistry` module and the GWB software give the same results.  (A final note: sometimes the `geochemistry` results differ from the Geochemists Workbench results in the 4$^{\mathrm{th}}$ significant figure.  This is because of the different precision used for the permittivity of free space, the Faraday constant, etc.)

!table id=table:surface caption=Surface characteristics
| Characteristic | pH=4 | pH=8 |
| --- | --- | --- |
| Surface charge ($\mu$C.cm$^{-2}$) | 16.0 | 0.4 |
| Surface potential (mV) | 168 | 17.1 |

!table id=table:weak_conc caption=Concentration of species on weak sites
| Site | Concentration (mmol.kg$^{-1}$) pH = 4 | Concentration (mmol.kg$^{-1}$) pH = 8 |
| --- | --- | --- |
| >(w)FeOH$_{2}^{+}$ | 1.23 | 0.129 |
| >(w)FeOH | 0.434 | 1.29 |
| >(w)FeO$^{-}$ | $0.35\times 10^{-2}$ | 0.295 |
| >(w)FeOHg$^{+}$ | $0.415\times 10^{-6}$ | 0.0984 |
| >(w)FeOPb$^{+}$ | $0.386\times 10^{-3}$ | 0.0534 |
| >(w)FeSO$_{4}^{-}$ | 0.117 | $0.189\times 10^{-3}$ |
| >(w)FeOHSO$_{4}^{2-}$ | 0.0825 | $0.377\times 10^{-2}$ |

!table id=table:strong_conc caption=Concentration of species on strong sites
| Site | Concentration (mmol.kg$^{-1}$) pH = 4 | Concentration (mmol.kg$^{-1}$) pH = 8 |
| --- | --- | --- |
| >(s)FeOH$_{2}^{+}$ | 0.00559 | $0.505\times 10^{-5}$ | 
| >(s)FeOH | 0.00197 |  $0.504\times 10^{-4}$ | 
| >(s)FeO$^{-}$ | $0.159\times 10^{-4}$ | $0.155\times 10^{-4}$ | 
| >(s)FeOHg$^{+}$ | $0.385\times 10^{-7}$ | $0.784\times 10^{-4}$ | 
| >(s)FeOPb$^{+}$ | 0.0392 | 0.0466 |

!table id=table:perc_sorbed caption=Percentage of species sorbed
| Species | % sorbed | % sorbed |
| --- | --- | --- |
| Hg$^{2+}$ | 0.000 | 98.45 |
| Pb$^{2+}$ | 39.59 | 100 |
| SO$_{4}^{2-}$ | 99.95 | 1.98 |

!bibtex bibliography