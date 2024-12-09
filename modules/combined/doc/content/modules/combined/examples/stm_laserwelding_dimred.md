# Using Stochastic Tools for the full-field reconstruction of multiphysics problems

The purpose of this example is to present a multiphysics model order reduction case using
the [Stochastic Tools Module](modules/stochastic_tools/index.md). The problem of interest is a
melt pool model using the [Navier-Stokes](modules/navier_stokes/index.md) module.
The problem involves multiple uncertain process parameters and are interested in predicting
full field quantities for unseen process parameter combinations using non-intrusive surrogate
models in combination with Proper Orthogonal Decomposition (POD).

## Problem Description

The problem of interest involves a transient 2D melt pool simulation for a 316L stainless steel
with a moving Gaussian laser beam. The laser source melts the top layer of the metal and
based on the equlibrium of the forces (surface tension, vapor recoil pressure) the surface of the
melt pool can deform considerably. The changing geometry is taken into account using an
Arbitrary Legrangian Eulerian (ALE) approach. The setup of the problem is depicted in [fig:geom].

!media combined/laser_weld/geometry.png caption=Problem setup for the laser melt pool simulation id=fig:geom style=width:100%

The temperature-dependent thermal properties of the problem have been taken from the
references listed in [tab:mat_prop].

!table caption=Material properties for 316L Stailess steel id=tab:mat_prop
| Property | Source|
| :- | :- |
| Dynamic viscosity | [!cite](kim1975thermophysical)$^\dagger$ |
| Density | [!cite](kim1975thermophysical) |
| Specific heat | [!cite](kim1975thermophysical) |
| Thermal conductivity | [!cite](pichler2022surface) |
| Surface tension | [!cite](pichler2022surface) |
| Vapor recoil pressure | [!cite](chen2021numerical) |

$\dagger$ In the treatment of dynamic viscosity, following the recommendations in [!cite](noble2007use),
we artificially cap the minimum value to stabilize the fluid flow solver.

The input file used for the simulations is presented below:

!listing examples/stochastic/laser_welding_dimred/2d.i caption=Laser meltpool model input file id=list:meltpool-ref

We see that it utilizes the `!include` syntax to read the process parameters togheter with the mesh and
kernel and boundary condition objects from other input files. This input file design reduces
duplication when it comes to the testing of the reduced-order models in subsequent sections.

!listing examples/stochastic/laser_welding_dimred/parameters.i caption=Input file for specifying process parameters id=list:parameters-ref

!listing examples/stochastic/laser_welding_dimred/mesh.i caption=Input file for specifying the mesh id=list:mesh-ref

!listing examples/stochastic/laser_welding_dimred/physics_objects.i caption=Input file adding the objects that define the physics id=list:physics-ref

### Process parameters

A total of two process parameters have been identified to be changable: the effective laser power and
the effective laser radius. They can change in the intervals shown in [tab:ppars] assuming a uniform
distribution.

!table caption=The distributions used for the process parameters id=tab:ppars
| Property | Min. | Max. |
| :- | - | - |
| Effective laser power  | 60 W | 74 W |
| Effective laser radius | 125 $\mu m$ | 155 $\mu m$ |

### Quantities of Interest

The quantity of interest in our case is the whole temperature field at the end of the simulation (0.4~ms).
To be able to reconstruct the whole field we utilize Proper Orthogonal Decomposition (POD) in conjunction
 with a Multi-output Gaussian Process
(MOGP).

## Results

## Reference results

Two reference results are presented here:
1. Results for a simulation with low effective laser power ($60~W$) and wide effective laser radius ($155~\mu m$)
2. Results for a simulation with high effective laser power ($74~W$) and narrow effective laser radius ($125~\mu m$)

The reference results are filtered to show the melt pool shape. We see that for the first case, the
steel melts, but no displacement is visible due to little to no evaporation. On the other hand, the
increase in laser power results in an increased evaporation resulting in significant surface deformation.

!media combined/laser_weld/60.png caption=Simulation results at the final time step with laser power ($60~W$) and wide effective laser radius ($155~\mu m$) id=fig:result-60 style=width:100%

!media combined/laser_weld/74.png caption=Simulation results at the final time step with laser power ($74~W$) and narrow effective laser radius ($125~\mu m$) id=fig:result-74 style=width:100%

## Training the reduced-order model

The reduced-order model was trained by first running the reference model with 45 different
process parameter combinations in a [Monte Carlo sampler](MonteCarloSampler.md). The input file used for training
is presented below.

!listing examples/stochastic/laser_welding_dimred/train.i id=list:training

The parts that require special attention are the snapshot gathering
parts in the transfer and reporter blocks.

!listing examples/stochastic/laser_welding_dimred/train.i caption=Transfers and Reporters needed for snapshot collection block=Transfers Reporters VariableMappings remove=Reporters/matrix Reporters/svd id=list:training-trans-rep

With the snapshots all available in one place, we can extract the POD modes
necessary for the dimensionality reduction. This is done using the
[PODMapping.md] reporter which not only creates the decomposition but also maps the
created snapshots onto a low-dimensional latent space.

!plot scatter
  id=results caption=Scree plot of the first 8 singular values extracted from the training data.
  filename=examples/stochastic/laser_welding_dimred/gold/sv_to_print.csv
  data=[{'x':'i', 'y':'sv', 'name':'Singular values'}]
  layout={'xaxis':{'type':'linear', 'title':'Mode #'},
          'yaxis':{'type':'log','title':'Singular Value (-)'}}

 In this specific example, the
dimensionality of the latent space was selected to be 8.
Once the coordinates of the snapshots are available in the latent space, we can fit a
[MOGP](GaussianProcessTrainer.md) on them. This is done in the `Trainers` block with the corresponding covariance functions defined in the `Covariance` block.

!listing examples/stochastic/laser_welding_dimred/train.i block=Trainers Covariance id=list:training-gp caption=Definition of the Gaussian Process surrogate used for fitting the coordinates in the latent space.

One can then save both the POD mapping and MOGP into restratable structures for
later usage.

!listing examples/stochastic/laser_welding_dimred/train.i block=Outputs

## Evaluating the error of the ROM

We evaluate the ROM by comparing reconstructed temperature fields at unseen process
parameter combinations to full-order model results at the same samples.
The metric for comparison is the relative $L^2$ error defined as:

\begin{equation}
e = \frac{||T_{ROM}-T_{FOM}||}{||T_{FOM}||}
\end{equation}

This error is generated in a new input file, which computes the reconstructed field and
solves the full-order model at the same time.

!listing examples/stochastic/laser_welding_dimred/2d-reconst.i caption=Test input file for comparing the reconstructed fields to the reference solutions id=list:2d-reconst

We see that duplication is minimized by reusing the same building blocks as the original input file.
In this case, we load both the MOGP and the POD mapping from a file:

!listing examples/stochastic/laser_welding_dimred/2d-reconst.i block=VariableMappings Surrogates

And use them in an object that reconstructs the field variable and inserts it into a `T_reconst`
auxiliary variable.

!listing examples/stochastic/laser_welding_dimred/2d-reconst.i block=UserObjects

Then, the integrated L2 error is computed by the appropriate postprocessor.

!listing examples/stochastic/laser_welding_dimred/2d-reconst.i block=Postprocessors

This input file is run for 90 samples of new process parameter combination selected by
a Monte Carlo sampler. The input file for automating the process of error analysis is presented below.

!listing examples/stochastic/laser_welding_dimred/test.i id=list:2d-reconst-pp caption=Input file for the automation of the testing phase.

This simply gathers the relative $L^2$ errors from the subapplications and
presents the results in a `json` file.
The error histogram is presented below. We see that the maximum integrated $L^2$
error is below 1.6% with the majority of the samples having error
in the 0-0.2% interval.


!plot histogram
  id=test-results caption=Histogram of the L2 errors over the 90 test samples.
  filename=examples/stochastic/laser_welding_dimred/gold/error_to_plot.csv
  data=[{'x':'l2error', 'name':'L2 Error'}]
  layout={'xaxis':{'type':'linear', 'title':'Relative L2 Error'},
          'yaxis':{'type':'linear','title':'Frequency'}}




