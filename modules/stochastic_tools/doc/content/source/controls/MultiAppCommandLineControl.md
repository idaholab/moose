# MultiAppCommandLineControl

!syntax description /Controls/MultiAppCommandLineControl

## Description

Depending on the type of stochastic analysis being performed, it may be necessary to pass command
line arguments to the MultiApp objects being created. For example, if a the domain is required to be
altered statistically in the x-direction the `Mesh/xmax` parameter in the sub-application must be
altered. However, this value cannot be altered from within sub-application input file via a Control
object because the `xmax` parameter is applied to the mesh when it is created. In this case, the
`MultiAppCommandLineControl` can be used to pass custom values to the arguments of a MultiApp that
are generated from Sampler and Distribution objects.

## Example Use

Consider a stochastic problem that is executing 3 simulations, but those simulations
require the maximum x and y coordinates of a generated mesh be varied between 5 and 10.

First, the MultiApps block is defined to execute the desired simulations.

!listing multiapps/commandline_control/master_multiple.i block=Distributions

Second, a [UniformDistribution](distributions/UniformDistribution.md)
object must be created:

!listing multiapps/commandline_control/master_multiple.i block=Distributions

A sampling scheme must be defined for capturing values from the uniform distribution. In
this example, since there are two pieces of data to be controlled ("xmax" and "ymax") the
uniform distribution is sampled twice. Since this sampled data will only be used during
"PRE_MULTIAPP_SETUP" execution, so the "execute_on" parameter is setup to match.

!listing multiapps/commandline_control/master_multiple.i block=Samplers

Finally, the `MultiAppCommandLineControl` is used to apply the sampled data to the
desired Mesh settings.

!listing multiapps/commandline_control/master_multiple.i block=Controls

!syntax parameters /Controls/MultiAppCommandLineControl

!syntax inputs /Controls/MultiAppCommandLineControl

!syntax children /Controls/MultiAppCommandLineControl

!bibtex bibliography
