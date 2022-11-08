# Simulating biogeochemistry

The `geochemistry` module assumes that microbes catalyze reactions (usually redox reactions) such as
\begin{equation}
\label{methanogen.cata}
\mathrm{CH}_{3}\mathrm{COO}^{-} \rightleftharpoons \mathrm{CH}_{4}\mathrm{(aq)} + \mathrm{HCO}_{3}^{-} - \mathrm{H}_{2}\mathrm{O} \ .
\end{equation}
The microbe-mediated reactions are usually governed by kinetic rates that are often more complicated than standard geochemical rates.  The [GeochemistryKineticRate](GeochemistryKineticRate.md) page documents the general form of kinetic rates, and readers should consult that page in tandem with this one.

Biogeochemical reactions are handled by introducing a new species into the database to represent the microbe.  There are two ways of doing this, as detailed in the following sections.

!alert note
It is important to read the following sections: which method suits your application will need careful consideration.

## Using a primary species to represent the microbe

The microbe species can be represented by a primary species, and the reactant (such as CH$_{3}$COO$^{-}$ in [methanogen.cata]) as a kinetic species.  The database that comes with the `geochemistry` module contains three such species, called `Biomass1`, `Biomass2`, `Biomass3`, but more can easily be added along the lines of:

```
    "Biomass1": {
      "elements": {},
      "radius": -1.5,
      "charge": 0.0,
      "molecular weight": 1.0
    },
```

Note that `radius = -1.5`, which means that the Debye-Huckel [activity coefficient](activity_coefficients.md) is always unity.  Also note that in this case the molecular weight is set to 1$\,$g.mol$^{-1}$.  This is important to remember when setting kinetic rate constants.

The advantage of using a primary species to represent a microbe is the simplicity: a new species is added to the database, and the kinetic rate is defined appropriately (a worked example is found below).  The main disadvantages are that microbe mortality cannot be represented and the kinetic rates cannot depend on the free molality of the kinetic species, only its total mole number (if these are important, a kinetic species must be used, as per the next section)

It is important to consider whether the reactant can accurately be treated as a kinetic species, or whether it is at equilibrium with the aqueous solution, in which case this method should not be used.

An example is found on the [sulfate reducer](bio_sulfate.md) page.

## Using a kinetic species to represent the microbe

In this case, the reactants and products of the catalysed equation are treated as species in equilibrium with the aqueous solution, and the microbe is a kinetic species that the `geochemistry` code handles separately.  It is more likely that this more accurately represents reality than the previous method.

To represent [methanogen.cata], the database could contain

```
    "methanogen": {
      "species": {
        "CH3COO-": -1.0,
        "H2O": -1.0,
        "CH4(aq)": 1.0,
        "HCO3-": 1.0
      },
      "molar volume": 1,
      "molecular weight": 1E9,
      "logk": [2.7272, 2.641, 2.699, 2.933, 3.28, 3.788, 4.336, 4.789]
    },
```

It is usually advantageous to place this in the `mineral species` section of the database, because microbes are often assumed to be immobile and have activity 1, just like minerals.  Otherwise, they could be treated as redox couples.

Studying the above database entry reveals the following.

1. The reactants in the original [methanogen.cata] (CH$_{3}$COO$^{-}$ in this case) must be brought to the right-hand side of the equation.  This has the interpretation that "when a methanogen reacts, it removes acetate and water to produce methane and biocarbonate", viz:

\begin{equation}
\mathrm{methanogen} \rightleftharpoons -\mathrm{CH}_{3}\mathrm{COO}^{-} + \mathrm{CH}_{4}\mathrm{(aq)} + \mathrm{HCO}_{3}^{-} - \mathrm{H}_{2}\mathrm{O} \ .
\end{equation}

2. The `molar volume` and `molecular weight` are largely unimportant.  Users should set reasonable numeric values, and keep those values in mind when defining the kinetic rates (below), since they impact the numeric values of the rate parameters.  In most cases, only the `intrinsic_rate_constant` is impacted: see the [mortality](tests_and_examples/bio_death.md) page for an example.

3. The equilibrium constants must be manually entered.  These may be derived from the existing database entries, or the [reaction balancing](reaction_balancing.md) functionality of the `geochemistry` module may be used.  Examples are presented on the [sulfate reductions](bio_sulfate.md) and [arsenate reduction](bio_arsenate.md) pages.  In the current case, the database contains:

```
    "CH3COO-": {
      "species": {
        "HCO3-": 2.0,
        "H+": 1.0,
        "O2(aq)": -2.0
      },
      ...
      "logk": [160.6192, 146.7487, 130.6352, 115.8131, 100.9856, 89.0757, 79.0857, 70.4395]
    },
    "CH4(aq)": {
      "species": {
        "H2O": 1.0,
        "H+": 1.0,
        "HCO3-": 1.0,
        "O2(aq)": -2.0
      },
      ...
      "logk": [157.892, 144.108, 127.936, 112.88, 97.706, 85.288, 74.75, 65.65]
    },
```

Subtracting the second from the first yields [methanogen.cata] and the equilibrium constants.  For instance $160.6192 - 157.892 = 2.7272$ is the first entry.

The advantage of using a kinetic species to represent a microbe is the flexibility, for instance, mortality can be represented as a kinetic rate (see the examples below).  Moreover, in many cases, it is likely that the reactants and products of the catalysed equation are at equilibrium with the aqueous solution, making this approach more suitable than the former approach.  The main disadvantage is the complexity of correctly entering the required information into the database.

The following microbes appear in the standard MOOSE geochemistry database, and more can be added using the procedure mentioned above:

- `sulfate_reducer` that catalyses the reaction CH$_{3}$COO$^{-}$ + SO$_{4}^{2-}$ $\rightarrow$ HCO$_{3}^{-}$ + HS$^{-}$.  The `sulfate_reducer` appears in the mineral species list, and has a molecular volume of 1$\,$cm$^{3}$.g$^{-1}$ and a molar mass of $10^{3}\,$g.mol$^{-1}$.
- `methanogen` that catalyses the reaction CH$_{3}$COO$^{-}$ + H$_{2}$O $\rightarrow$ CH$_{4}$(aq) + HCO$_{3}^{-}$.  The `methanogen` appears in the mineral species list, and has a molecular volume of 1$\,$cm$^{3}$.g$^{-1}$ and a molar mass of $10^{9}\,$g.mol$^{-1}$.
- `arsenate_reducer` that catalyses the reaction Lactate$^{-}$ + 2HAsO$_{4}^{2-}$ + 2H$_{2}$O $\rightarrow$ CO$_{3}^{2-}$ + CH$_{3}$COO$^{-}$ + As(OH)$_{4}^{-}$.  The `arsenate_reducer` appears as a redox couple, with a `radius` of -1.5, meaning that its activity will always be unity, and its molar mass is set to 1$\,$g.mol$^{-1}$.

The molar masses are definitely incorrect, but they are only important to remember when setting the kinetic rates.  The examples below discuss this in detail.

The following are worked examples:

- [Simulating mortality](tests_and_examples/bio_death.md)
- [Sulfate reducer example](tests_and_examples/bio_sulfate.md)
- [Arsenate reducer example](tests_and_examples/bio_arsenate.md)
- [Zoning in an aquifer](bio_zoning.md) (a reactive-transport model)
