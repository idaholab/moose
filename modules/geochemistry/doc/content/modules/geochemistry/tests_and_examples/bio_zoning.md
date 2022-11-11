# Reactive transport with biogeochemistry: aquifer zoning

This follows the example in Section 33.2 of [!cite](bethke_2007).  Groundwater flows through an aquifer from west to east (towards the positive "$x$" direction).  The aquifer is 200$\,$km long, has a porosity of 0.3, and the flow rate is 10$\,$m$^{3}$(water).m$^{-2}$(rock).yr$^{-1}$.  The groundwater has pH 7.5, and contains 1$\,$mmolal Ca$^{2+}$, 2$\,$mmolal HCO$_{3}^{-}$, 40$\,\mu$molal SO$_{4}^{2-}$, and small amounts of acetate, sulfide and methane.  Living in the aquifer are two species of microbe: sulfate reducers and methanogens, which are discussed in detail below.  The [model definition](GeochemicalModelDefinition) is:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_conc.i block=definition

## Description of biogeochemistry

The two species of microbe are: sulfate reducers that catalyse the reaction
\begin{equation}
\label{sulfate.reduction.eqn}
\mathrm{acetate}^{-} + \mathrm{SO}_{4}^{2-} \rightarrow 2\mathrm{HCO}_{3}^{-} + \mathrm{HS}^{-} \ ;
\end{equation}
and methanogens that catalyse the reaction
\begin{equation}
\label{meth.eqn}
\mathrm{acetate}^{-} + \mathrm{H}_{2}\mathrm{O} \rightarrow \mathrm{CH}_{4}\mathrm{(aq)} + \mathrm{HCO}_{3}^{-} \ .
\end{equation}
Acetate (CH$_{3}$COO$^{-}$) is added to the aquifer from surrounding rocks at a rate of $4\,\mu$mol.m$^{-3}$.yr$^{-1}$ (the m$^{3}$ in the denominator is a volume of rock, not a volume of groundwater) which is a rate consistent with observations.  This feeds the microbes.  The initial microbial population is small.

The standard MOOSE geochemistry database contains the necessary entries (see the [biogeochemistry](theory/biogeochemistry.md) page for more discussion):

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
      "logk": [2.727, 2.641, 2.699, 2.933, 3.28, 3.788, 4.336, 4.789]
    },
    "sulfate_reducer": {
      "species": {
        "CH3COO-": -1.0,
        "SO4--": -1.0,
        "HCO3-": 2.0,
        "HS-": 1.0
      },
      "molar volume": 1,
      "molecular weight": 1E3,
	"logk": [8.502, 8.404, 8.451, 8.657, 9.023, 9.457, 9.917, 10.31]
    },
```

The [rate](GeochemistryKineticRate.md) of [sulfate.reduction.eqn] is assumed to be of the form
\begin{equation}
\label{eqn.rrs}
r = n_{w}k_{+}C_{\mathrm{sr}} \frac{m_{ac}}{m_{ac} + K_{ac}} \frac{m_{SO4}}{m_{SO4} + K_{SO4}} \left(1 - \left(\frac{Q \exp(45000/RT)}{K} \right)^{1/5} \right) \ .
\end{equation}
where

- $n_{w}$ (units: kg) is the mass of solvent water;
- $k_{+} = 10^{-9}\,$mol.mg$^{-1}$.s$^{-1} = 31.536\,$mol.g$^{-1}$.year$^{-1}$ is the intrinsic rate constant, where the mass (grams) in the denominator is the mass of the sulfate-reducer, and the direction is forward (dissolution of acetate only, not the reverse);
- $C_{\mathrm{sf}}$ (units: g.kg$^{-1}$) is the concentration of the sulfate-reducing microbe, where the mass (kg) in the denominator is the mass of solvent water;
- $m_{ac}$ and $m_{SO4}$ (units: mol.kg$^{-1}$) are the molality of acetate and SO$_{4}^{2-}$, respectively (these are free concentrations, not the bulk compositions);
- $K_{ac} = 70\times 10^{-6}\,$mol.kg$^{-1}$ and $K_{SO4} = 200\times 10^{-6}\,$mol.kg$^{-1}$ are the half-saturation constant of the acetate and SO$_{4}^{2-}$, respectively;
- $Q$ is the activity product of [sulfate.reduction.eqn];
- $K$ is the equilibrium constant of [sulfate.reduction.eqn];
- $45000\,$J.mol$^{-1}$ is an estimate of the energy captured by the microbe for each mole turnover of [sulfate.reduction.eqn];
- $R=8.314472\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant;
- $T$ (units: K) is the temperature.

In addition, the growth yield of the sulfate reducer is assumed to be 4.3$\,$g.mol$^{-1}$.  The `sulfate_reducer` has a molar mass of 1000$\,$g/mol, so `kinetic_biological_efficiency = 4.3E-3`.

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_conc.i block=rate_sulfate_reducer

The [rate](GeochemistryKineticRate.md) of [meth.eqn] is assumed to be of the form
\begin{equation}
\label{eqn.rmeth}
r = n_{w}k_{+}C_{\mathrm{meth}} \frac{m_{ac}}{m_{ac} + K_{ac}} \left(1 - \left(\frac{Q \exp(24000/RT)}{K} \right)^{1/2} \right) \ .
\end{equation}
where

- $n_{w}$ (units: kg) is the mass of solvent water;
- $k_{+} = 2\times 10^{-9}\,$mol.mg$^{-1}$.s$^{-1} = 63.072\,$mol.g$^{-1}$.year$^{-1}$ is the intrinsic rate constant, where the mass (grams) in the denominator is the mass of methanogen, and the direction is forward (dissolution of acetate only, not the reverse);
- $C_{\mathrm{meth}}$ (units: g.kg$^{-1}$) is the methanogen concentration, where the mass (kg) in the denominator is the mass of solvent water;
- $m_{ac}$ (units: mol.kg$^{-1}$) is the molality of acetate (this is free concentration, not bulk composition);
- $K_{ac} = 20\times 10^{-6}\,$mol.kg$^{-1}$ is the half-saturation constant of the acetate
- $Q$ is the activity product of [meth.eqn];
- $K$ is the equilibrium constant of [meth.eqn];
- $24000\,$J.mol$^{-1}$ is an estimate of the energy captured by the microbe for each mole turnover of [meth.eqn];
- $R=8.314472\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant;
- $T$ (units: K) is the temperature.

In addition, the growth yield of the sulfate reducer is assumed to be 2$\,$g.mol$^{-1}$.  The `methanogen` has a molar mass of $10^{9}\,$g/mol, so `kinetic_biological_efficiency = 2.0E-9`.

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_conc.i block=rate_methanogen

In addition, both species of microbe are assumed to die at a rate of $10^{-9}\,$s$^{-1}$, which is implemented in the following [GeochemistryKineticRate](GeochemistryKineticRate.md) userobjects:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_conc.i block=death_sulfate_reducer

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_conc.i block=death_methanogen

To avoid the accumulation of HS$^{-}$ from the sulfate reducers, the aquifer is assumed to include small amounts of siderite minerals, which reaction to form mackinawite:
\begin{equation}
\mathrm{siderite} + \mathrm{HS}^{-} \rightarrow \mathrm{mackinawite} + \mathrm{HCO}_{3}^{-}
\end{equation}

The remainder of the geochemistry is standard, except for the `source_species_rates` that encode the groundwater flow that are sent from the flow App described below:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_conc.i

## Flow

The operator-split method is used to couple the flow with the biogeochemistry, as discussed in the [theory](theory/index.md) page.  Since porosity does not change in this simulation, and the groundwater velocity is specified, only the groundwater (not the surrounding rock) is considered.  The flow is implemented using [GeochemistryTimeDerivative](GeochemistryTimeDerivative.md) and [ConservativeAdvection](ConservativeAdvection.md) Kernels, acting on the mole number per litre of groundwater of each of the chemical components that flow through the aquifer:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_flow.i block=Kernels

The rate of change of each component is saved into an `AuxVariable`, for instance `rate_H2O_times_vv`.  This has units mol.yr$^{-1}$, because the `Variables` represent the number of moles per litre of groundwater of each component at each node.  To convert it to mol.yr$^{-1}$.litre$^{-1}$(groundwater), a `ParsedAux` is used, such as:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_flow.i block=rate_H2O_auxk

Fresh groundwater is added at the left, by boundary conditions such as:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_flow.i block=inject_H2O

At the end of each flow timestep, the geochemistry solver is run:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_flow.i block=MultiApps

with the flow rates from the flow solver acting as source terms to the geochemistry simulation:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_flow.i block=changes_due_to_flow

After the geochemistry solver has completed its timestep, the new concentrations are retrieved from the geochemistry model in preparation for the next flow timestep:

!listing modules/geochemistry/test/tests/kinetics/bio_zoning_flow.i block=transported_moles_from_geochem









## Results

As seen in [bio_zoning.mov], the simulation gradual evolves to steady-state in which the aquifer is partitioned into two distinct zones: the upstream zone is inhabited by the sulfate reducers, who drive the sulfate concentration to low values by around 100$\,$km from the fresh groundwater source (which sits at $x=0$).  The downstream zone is inhabited by methanogens, who produce significant dissolved methane.

!media geochemistry/bio_zoning.mp4 caption=Results of the aquifer-zoning biogeochemistry reactive transport simulation  id=bio_zoning.mov








