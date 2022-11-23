# The `geochemistry` module: tests and examples

The theory behind the `geochemistry` module has been described in the [theory page](theory/index.md).  It is hoped that the descriptions of tests and examples found below will help users actually implement their geochemical models.

The `geochemistry` module's code is accompanied by over 350 tests, ranging from simple unit tests to more complicated benchmarks such as some of those listed below.  Code coverage details may be found [here](https://mooseframework.inl.gov/docs/coverage/geochemistry/index.html).

Many of the tests and examples are drawn from the popular textbook [!cite](bethke_2007).  The textbook is not reproduced here, so if readers are interested in an example's background, details or an analysis of the results they should refer to [!cite](bethke_2007).  To help new `geochemistry` module users, many tests and examples are also provided with a [Geochemists Workbench](https://www.gwb.com/) (GWB) equivalent.  GWB is a "gold-standard" geochemistry solver so its results also act to benchmark the `geochemistry` module.  There are [small input-file related differences](theory/gwb_diff.md) bewteen GWB and `geochemistry` that must be considered when benchmarking to full precision, but in real models their impact is likely to be much less than the errors in the database and experimental observations.

## Equilibrium models of various waters

- [A simple HCl solution](HCl.md)
- [Chemical model of seawater](seawater.md)
- [Water in the Amazon river](amazon.md)
- [Red Sea brine](red_sea.md)

## Models with redox disequilibrium

- [Morro de Ferro groundwater](morro.md)
- [Microbial respiration](microbial_redox.md)

## Exploring solubility and activities

- [Solubility of gypsum](gypsum.md)
- [Saturation of halite and anhydrite at Sebkhat El Melah](sebkhat.md)

## Sorption and surface complexation

- [Langmuir sorption of selenate in loamy soil](selenate.md)
- [A problem involving surface complexation](surface_complexation.md)

## Tasks involving reaction balancing

- [Reactions in terms of different components](reaction_balancing.md)
- [Equilibrium activity ratios](activity_ratios.md)
- [Computing equilibrium pH](pH_pe.md)
- [Equilibrium temperature or activity](eqm_temp_a.md)

## Nonunique solutions

- [Models that exhibit nonunique solutions](non_unique.md)

## Simple time-dependent reaction paths

- [Progressively adding chemicals](adding_feldspar.md)
- [Progressively changing the temperature](cooling_feldspar.md)
- [Progressively adding chemicals with fixed fugacity](adding_pyrite.md)
- [Progressively changing fugacity](changing_fugacity_with_calcite.md)
- [Progressively changing pH](changing_pH_iron.md)
- [Adding fluids of different temperatures](pickup.md)
- [Dumping minerals then adding chemicals](calcite_buffer.md)
- [Flow-through reactions that remove minerals](flow_through.md)
- [Flushing minerals](flush.md)

## Kinetics

- [Gradual dissolution of quartz](kinetic_quartz.md)
- [Dissolution of albite into an acidic solution](kinetic_albite.md)
- [Quartz deposition in a fracture](kinetic_quartz_arrhenius.md)

## Reactive transport

- [A 2D GeoTES simulation](geotes_2D.md)
- [A GeoTES simulation involving the Weber-Tensleep formation](geotes_weber_tensleep.md)
- [A geothermal simulation involving 2D flow](forge.md)
- [Zoning in an aquifer](bio_zoning.md)

## Biogeochemistry

- [Basics](theory/biogeochemistry.md)
- [Simulating mortality](bio_death.md)
- [A sulfate reducer in the presence of acetate](bio_sulfate.md)
- [Arsenate reduction in the presence of lactate](bio_arsenate.md)
- [Zoning in an aquifer: a reactive-transport model](bio_zoning.md)


!bibtex bibliography