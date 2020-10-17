# DiscreteNucleationInserter

!syntax description /UserObjects/DiscreteNucleationInserter

The inserter manages the global list of currently active nucleus stabilization sites. The inserter object keeps track if any changes to the nucleus list occurred in the current timestep.


This user object takes two required parameters

- `hold_time` - the duration in time for which a stabilization site remains active
- `probability` - a material property containing a nucleation rate density. This material property can be calculated using classical nucleation theory for example.

In addition, the user has the option of specifying whether this nuclei with a fixed
radius will be inserted, or whether the radius can vary based on an internal calculation. A
constant radius can be supplied via the input file to the `radius` parameter, while a variable
radius can be used by providing a material property name to `radius` (a default name is provided).

The final parameter, `time_dependent_statistics`, is a boolean that defaults to true.  This
parameter indicates whether time-dependent nucleation statistics are used or not.  
The default of true indicates that time-dependent statistics are used, i.e., that the nucleation
probability depends on time step size. A value of false indicates time-independent
nucleation statistics.  This might be used in a situation in which the physics driving
nucleus formation occur at much faster time scales than the other physics in microstructure
evolution.  For example, grain boundary motion during recrystallization is much slower
than dislocation motion to organize into cells or the nuclei of new grains.  As a result,
time-independent statistics can be applied.

The `DiscreteNucleationInserter` is part of the [Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /UserObjects/DiscreteNucleationInserter

!syntax inputs /UserObjects/DiscreteNucleationInserter

!syntax children /UserObjects/DiscreteNucleationInserter
