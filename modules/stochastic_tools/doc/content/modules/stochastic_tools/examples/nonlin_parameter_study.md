# Parameter Study on a Highly Nonlinear Problem

This example assumes that the reader has already visited the example in [examples/parameter_study.md] and is familiar
with the fundamental blocks used in parent input files.
In this example, the effect of varying the distribution of the uncertain
parameters on the distribution of the Quantities of Interest (QoIs) is showcased as well.   

## Problem Description

The strong formulation of the problem is taken from [!cite](chaturantabut2010nonlinear) and can be written as

!equation id=nonlin-diff-react-strong
-\Delta u + \frac{\mu_1}{\mu_2}\left(e^{\mu_2 u}-1\right) = 100 \sin(2\pi x)\sin(2\pi y),

where $u$ is a scalar field variable, $(x,y)\in[0,1]\times[0,1]$ are the physical coordinates,
while $\mu_1$ and $\mu_2$ are uncertain parameters with known (or assumed) probability distributions.
This equation is supplemented with homogeneous Dirichlet boundary conditions on every side
of the $[0,1]\times[0,1]$ square.

## Solution of the Problem

To be able to perform a parameter study, the application has to be able to solve the
problem with fixed parameters first. The input file used for this purpose is provided in [nonlin_diff_react].
The nominal values of the uncertain parameters are $\mu_{1,n}=0.3$ and $\mu_{2,n}$=9 in this case.
There are two blocks in the input file that are worth examining in detail.
The first is the `Kernels` block that shows that a custom test kernel has been implemented to be
able to handle the exponential reaction term in [nonlin-diff-react-strong].  
To use this kernel, the user has to add an additional argument for the
Stochastic Tools executioner as follows:

```
cd moose/modules/stocastic_tools/examples/parameter_study/nonlin_diff_react
../../../stocastic_tools-opt -i nonlin_diff_react_sub.i --allow-test-objects
```

The second atypical block is `Controls` which is necessary to set up a channel
to receive and substitute new parameter samples from the parent application.
As shown in the `Postprocessors` block, the Quantities of Interest (QoIs) are the
maximum value ($u_{max}$), minimum value ($u_{min}$) and the average value ($u_{avg}$)
of the scalar field variable $u$.

!listing parameter_study/nonlin_diff_react/nonlin_diff_react_sub.i id=nonlin_diff_react
         caption=Complete input file for the nonlinear problem using the nominal values of the uncertain parameters.

## Parent application Input

As described in [parameter_study.md] in detail, one needs a driver input (or parent input)
to perform a parameter study.
Two parent input files are provided for this example in [nonlin_diff_react_parent_uniform]
and [nonlin_diff_react_parent_normal]. The first considers the uncertain parameters to be
uniformly distributed around their nominal values
, $\mu_i\sim\mathcal{U} (0.7\mu_{i,n},1.3\mu_{i,n})$, while the second one assumes normal
distribution $\mu_i\sim\mathcal{N} (\mu_{i,n},0.15\mu_{i,n})$.
The only difference between the two input files is the `Distributions` block where the
assumed probability distributions are defined for the uncertain parameters.

!listing parameter_study/nonlin_diff_react/nonlin_diff_react_parent_uniform.i id=nonlin_diff_react_parent_uniform
         caption=Complete input file for the driver of the parameter study with uniformly distributed uncertain parameters.

!listing parameter_study/nonlin_diff_react/nonlin_diff_react_parent_normal.i block=Distributions id=nonlin_diff_react_parent_normal
         caption=Complete input file for the driver of the parameter study with normally distributed uncertain parameters.

For the sampling of the uncertain parameters, a Latin Hypercube Sampling (LHS) strategy is utilized.
Altogether 5000 parameter samples are created for the model.
Furthermore, the parent application is executed in a "batch-restore" mode, which provides a memory-efficient
way to run sub-applications. For the comparison between different running modes
the interested reader is referred to [stochastic_tools/batch_mode.md].

The objects in the `Transfers` block are responsible for the communication between the
parent and sub-applications. It streams parameter samples to sub-applications and
receives the corresponding values for the selected QoIs.
It is visible that in this example the type of the parameter transfer object is
[SamplerParameterTransfer.md] which streams the parameter samples to a [SamplerReceiver.md]
object (in `Controls` block) in the sub-application. This object then plugs the
new parameter values into kernels, materials or boundary conditions.
Unfortunately, this requires the parameters to be controllable in the sub-application,
which might not be true in every case.
For this specific example, the controllability of the parameters in `ExponentialReaction` kernel
is ensured by the last two commands in the `validParams` function.

!listing ExponentialReaction.C re=InputParameters\sExponentialReaction::validParams.*?^}

If the target parameters are not controllable, one can use a command line based communication
between parent and sub-applications. For more information about this approach see the example
covered in [poly_chaos_surrogate.md].

To run the parent application, it is still necessary to enable test objects using the following
command

```
cd moose/modules/stocastic_tools/examples/parameter_study/nonlin_diff_react
../../../stocastic_tools-opt -i nonlin_diff_react_parent_uniform.i --allow-test-objects
```


## Stochastic Results id=results

The distributions of the QoIs for the uniformly distributed uncertain parameters are
presented in [results_u_min_uniform], [results_u_max_uniform] and [results_u_avg_uniform].
The same distributions with normally distributed uncertain parameters are shown in
[results_u_min_normal], [results_u_max_normal] and [results_u_avg_normal].

The estimated mean values of the QoIs with the corresponding confidence intervals
are presented below assuming uniformly distributed parameters:

$\overline{u}_{min} = -1.3481,\,95\%\, CI[-1.3485, -1.3476]$

$\overline{u}_{max} = 0.8406,\,95\%\, CI[0.8383, 0.8430]$

$\overline{u}_{avg} = -0.1318,\,95\%\, CI[-0.1325, -0.1311]$

The same statistics for the normally distributed parameters are the following:

$\overline{u}_{min} = -1.3486,\,95\%\, CI[-1.3490, -1.3482]$

$\overline{u}_{max} = 0.8379,\,95\%\, CI[0.8359, 0.8400]$

$\overline{u}_{avg} = -0.1326,\,95\%\, CI[-0.1332, -0.1319]$

!plot histogram filename=stochastic_tools/examples/parameter_study/nonlin_diff_react/gold/nonlin_diff_react_parent_uniform_out_results_0002.csv
                vectors=results:min
                bins=50
                xlabel=Minimum value
                id=results_u_min_uniform
                caption=Resulting distribution of $u_{min}$ with uniformly distributed parameters.


!plot histogram filename=stochastic_tools/examples/parameter_study/nonlin_diff_react/gold/nonlin_diff_react_parent_uniform_out_results_0002.csv
                vectors=results:max
                bins=50
                xlabel=Maximum Value
                id=results_u_max_uniform
                caption=Resulting distribution of $u_{max}$ with uniformly distributed parameters.


!plot histogram filename=stochastic_tools/examples/parameter_study/nonlin_diff_react/gold/nonlin_diff_react_parent_uniform_out_results_0002.csv
                vectors=results:average
                bins=50
                xlabel=Average Value
                id=results_u_avg_uniform
                caption=Resulting distribution of $u_{avg}$ with uniformly distributed parameters.

!plot histogram filename=stochastic_tools/examples/parameter_study/nonlin_diff_react/gold/nonlin_diff_react_parent_normal_out_results_0002.csv
                vectors=results:min
                bins=50
                xlabel=Minimum value
                id=results_u_min_normal
                caption=Resulting distribution of $u_{min}$ with normally distributed parameters.


!plot histogram filename=stochastic_tools/examples/parameter_study/nonlin_diff_react/gold/nonlin_diff_react_parent_normal_out_results_0002.csv
                vectors=results:max
                bins=50
                xlabel=Maximum Value
                id=results_u_max_normal
                caption=Resulting distribution of $u_{max}$ with normally distributed parameters.


!plot histogram filename=stochastic_tools/examples/parameter_study/nonlin_diff_react/gold/nonlin_diff_react_parent_normal_out_results_0002.csv
                vectors=results:average
                bins=50
                xlabel=Average Value
                id=results_u_avg_normal
                caption=Resulting distribution of $u_{avg}$ with normally distributed parameters.
