# MultiAppSamplerControl

!syntax description /Controls/MultiAppSamplerControl

## Description

Depending on the type of stochastic analysis being performed, it may be necessary to pass command
line arguments to the MultiApp objects being created. For example, if a the domain is required to be
altered statistically in the x-direction the `Mesh/xmax` parameter in the sub-application must be
altered. However, this value cannot be altered from within sub-application input file via a Control
object because the `xmax` parameter is applied to the mesh when it is created. In this case, the
`MultiAppSamplerControl` can be used to pass custom values to the arguments of a MultiApp that
are generated from Sampler and Distribution objects.

## Example Use

Consider a stochastic problem that is executing 3 simulations, but those simulations
require the maximum x and y coordinates of a generated mesh be varied between 5 and 10.

First, the MultiApps block is defined to execute the desired simulations.

!listing multiapps/commandline_control/parent_multiple.i block=Distributions

Second, a [Uniform](distributions/Uniform.md) distribution
object must be created:

!listing multiapps/commandline_control/parent_multiple.i block=Distributions

A sampling scheme must be defined for capturing values from the uniform distribution. In
this example, since there are two pieces of data to be controlled ("xmax" and "ymax") the
uniform distribution is sampled twice. Since this sampled data will only be used during
"PRE_MULTIAPP_SETUP" execution, so the "execute_on" parameter is setup to match.

!listing multiapps/commandline_control/parent_multiple.i block=Samplers

Finally, the `MultiAppSamplerControl` is used to apply the sampled data to the
desired Mesh settings.

!listing multiapps/commandline_control/parent_multiple.i block=Controls

## Vector Parameter

The vector parameter can be altered statistically with `MultiAppSamplerControl`. To illustrate its usage, we consider an input file listed below:

!listing multiapps/batch_commandline_control/parent_vector.i block=Controls

In this input file, the `param_names` includes a vector parameter with 4 entries called `Materials/const/prop_values` and two scalar parameters called `Mesh/xmax` and `Mesh/ymax`.

Several cases exist for modifying the vector parameter:

1. All four entries will be altered: set `param_names = Materials/const/prop_values[0,1,2,3]`. The `[0,1,2,3]` is the global column index of the provided distributions which implies that each entry corresponds to a different distribution.  

2. The third and fourth entry will be altered with a same distribution: set `param_names = Materials/const/prop_values[0,1,2,2]`. The repeated index "2" means that their values are the same and will be sampled from a same distribution.

3. The second entry will not be altered: set `param_names = Materials/const/prop_values[0,(0.5),1,2]`. In this case, the second entry will be set as 0.5 while the first, third and last entries will be altered statistically. In general, a constant value will be provided inside the parentheses bracket.

!alert note
If `[]` is used, it must be provided to every parameter. By default, if the `[]` is not provided it is assumed that values are scalar and captured in order.

!syntax parameters /Controls/MultiAppSamplerControl

!syntax inputs /Controls/MultiAppSamplerControl

!syntax children /Controls/MultiAppSamplerControl

!bibtex bibliography
