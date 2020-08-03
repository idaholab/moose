# The `geochemistry` module: tests and examples

The theory behind the `geochemistry` module has been described mostly in the [equilibrium](equilibrium.md) and [transport](transport.md) pages.  It is hoped that the descriptions of tests and examples found below will help users actually implement their geochemical models.

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


!bibtex bibliography