# Biogeochemistry with an arsenate reducer

This follows the example in Section 33.1 of [!cite](bethke_2007).  Suppose that an arsenate-reducing microbe acts in the presence of lactate to catalyse the reaction
\begin{equation}
\label{arsenate_reduction.eqn}
\mathrm{lactate}^{-} + 2\mathrm{HAsO}_{4}^{2-} + 2\mathrm{H}_{2}\mathrm{O} \rightarrow \mathrm{CO}_{3}^{2-} + \mathrm{acetate}^{-} + 2\mathrm{As(OH)}_{4}^{-} \ .
\end{equation}
Bethke provides some background information.  In this situation, thermodynamic factors are the main control of the reaction rate: as the reaction proceeds and products accumulate, the energy liberated by the reaction reduces to the energy needed to synthesize cellular ATP, and the thermodynamic drive reduces to zero.

The standard MOOSE geochemical database contains the information:

```
    "arsenate_reducer": {
      "species": {
        "Lactate-": -1.0,
        "HAsO4--": -2.0,
        "H2O": -2.0,
        "CO3--": 1.0,
        "CH3COO-": 1.0,
        "As(OH)4-": 2.0
      },
      "charge": 0.0,
      "radius": -1.5,
      "molecular weight": 1,
	"logk": [6.742, 26.75, 49.93, 71.18, 92.42, 109.5, 123.8, 136.1]
    },
```

This appears in the `redox couples` section, but it could equally appear in the `mineral species` section.  The main [biogeochemistry](theory/biogeochemistry.md) page discusses this further.  If this entry did not appear in the database, it could be manually entered, but the equilibrium constants would have to be derived from those already in the database (or from experimental measurements).  This is easily done by using the [GeochemicalModelInterogator](GeochemicalModelInterrogator.md):

!listing modules/geochemistry/test/tests/kinetics/bio_arsenate0.i block=GeochemicalModelInterrogator

and the appropriate model definition:

!listing modules/geochemistry/test/tests/kinetics/bio_arsenate0.i block=UserObjects

In this example, the microbe is treated as a kinetic species, and all other species (lactate, etc) are at equilibrium.  The main [biogeochemistry](theory/biogeochemistry.md) page discusses this further.  Hence, the [GeochemicalModelDefinition](GeochemicalModelDefinition.md) is:

!listing modules/geochemistry/test/tests/kinetics/bio_arsenate1.i block=definition

The [rate](GeochemistryKineticRate.md) of [arsenate_reduction.eqn] is assumed to be of the form
\begin{equation}
\label{eqn.rr}
r = n_{w}k_{+}C_{\mathrm{biomass}} \frac{m}{m + K_{A}} \left(1 - \left(\frac{Q \exp(125000/RT)}{K} \right)^{1/4} \right) \ .
\end{equation}
(this is simpler than Bethke's form, although the results are almost the same) where

- $n_{w}$ (units: kg) is the mass of solvent water;
- $k_{+} = 7\times 10^{-9}\,$mol.mg$^{-1}$.s$^{-1} = 0.6048\,$mol.g$^{-1}$.day$^{-1}$ is the intrinsic rate constant, where the mass (grams) in the denominator is the mass of microbe, and the direction is forward (dissolution of acetate only, not the reverse);
- $C_{\mathrm{biomass}}$ (units: g.kg$^{-1}$) is the microbe concentration, where the mass (kg) in the denominator is the mass of solvent water;
- $m$ (units: mol.kg$^{-1}$) is the molality of HAsO$_{4}^{2-}$ (this is the free concentration, not the bulk composition);
- $K_{A} = 10\times 10^{-6}\,$mol.kg$^{-1}$ is the half-saturation constant of the acetate;
- $Q$ is the activity product of [arsenate_reduction.eqn];
- $K$ is the equilibrium constant of [arsenate_reduction.eqn];
- $125000\,$J.mol$^{-1}$ is an estimate of the energy captured by the microbe for each mole turnover of [arsenate_reduction.eqn];
- $R=8.314472\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant;
- $T$ (units: K) is the temperature.

In this example, microbe mortality is unimportant.  For each mole turnover of this reaction, Bethke estimates the microbe mass increases by $5\,$g.

The `arsenate_reducer` species has a molar mass of $1\,$g.mol$^{-1}$ in the database file.  It is unimportant that this is definitely incorrect: it is only important to remember this value when entering numerical values for the kinetic rate, below.

[eqn.rr] is implemented using the following [GeochemistryKineticRate](GeochemistryKineticRate.md) UserObject:

!listing modules/geochemistry/test/tests/kinetics/bio_arsenate1.i block=rate_arsenate_reducer

Most of the values are self-explanatory.  Note:

- `multiply_by_mass = true` means the rate is multiplied by the mass (in grams) of the `arsenate_reducer`, as desired
- there is no need to use H2O in the `promoting_species` because $n_{w}C_{\mathrm{biomass}}$ is just the mass of the `sulfate_reducer` in grams
- `kinetic_biological_efficiency = 5` because this is the number of moles of `arsenate_reducer` that is created for one mole [arsenate_reduction.eqn] turnover: recall that its molar mass is $1\,$g.mol$^{-1}$


## Results

As seen in [bio_arsenate.fig], the microbe mass initially grows rapidly, along with the total reaction rate, but after about 1.3 days, too many reaction products have accumulated and the thermodynamic drive reduces to zero.

!media bio_arsenate.png caption=Results of the biologically-catalysed arsenate reduction  id=bio_arsenate.fig








