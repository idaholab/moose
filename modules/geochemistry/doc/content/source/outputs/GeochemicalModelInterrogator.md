# GeochemicalModelInterrogator

Usually this Output is added using the [GeochemicalModelInterrogator action](GeochemicalModelInterrogator/index.md).  The purpose of this Output is to output [balanced reactions and equilibrium constants](reaction_balancing.md), [activity ratios](activity_ratios.md), [equilibrium temperature](eqm_temp_a.md), etc, depending on the `interrogation` option.  If a invalid swap is defined through the `swap_into_basis` and `swap_out_of_basis` options, the simulation will exit with an explanatory error message.

An example of outputting equilibrium reactions (`interrogation = reaction`) for different choices of basis components is

!listing modules/geochemistry/test/tests/interrogate_reactions/clinoptilolite.i

An example of outputting activity ratios/products (`interrogation = activity`) is

!listing modules/geochemistry/test/tests/interrogate_reactions/muscovite.i

An example of computing temperature (`interrogation = eqm_temperature`) at equilibrium is

!listing modules/geochemistry/test/tests/interrogate_reactions/gypsum.i

An example of outputting pH and pe (`interrogation = activity`) at equilibrium is

!listing modules/geochemistry/test/tests/interrogate_reactions/hematite.i


!syntax parameters /Outputs/GeochemicalModelInterrogator

!syntax inputs /Outputs/GeochemicalModelInterrogator

!syntax children /Outputs/GeochemicalModelInterrogator
