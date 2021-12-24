# Material Inversion Example

In a material inversion problem, inverse optimization is used to tune the material parameters to match the experimental data.  The examples in this section are for steady state heat conduction where material inversion will be used to determine thermal conductivity.  Material inversion problems result in a nonlinear optimization problem and are difficult to solve.  To make these problems easier for the optimization algorithms to solve, a close initial guess and tight bounds for the thermal conductivity are needed.  The theory for PDE constrained inverse optimization and its application thermal problems are available in the [theory section](getting_started/InvOptTheory.md).  Two material inversion problems will be presented in this section for steady state heat conduction where the following types of loadings are parameterized:

- Example 1: Constant Thermal Conductivity

- Example 2: Temperature Dependent Thermal Conductivity

## Example 1: Constant Thermal Conductivity

In this example we are parameterizing the heat source intensity at the locations indicated by the $\bigcirc$ symbols in [figSetup] that will produce a temperature field that most closely matches the temperature measurements taken at the points indicated by the $\times$ symbols.  Dirichlet boundary conditions are applied to the entire boundary with T=300.  

!media media/fig_setup.png
       style=width:40%;margin:auto;padding-top:2.5%;
       id=figSetup
       caption=Meshed geometry with measurement ($\times$) and parameterized point load ($\bigcirc$) locations.

## Example 2: Temperature Dependent Thermal Conductivity

Optimization problems are solved using the [MultiApps] system.  The main application contains the optimization executioner and the sub-applications solve the forward and adjoint PDE.   The main application input is shown in [master_app].

!listing test/tests/formfunction/objective_gradient_minimize/point_loads/master.i
         id=master_app
         caption=Main application optimization input for point load parameterization shown in [figSetup]

The main application runs the optimization executioner and transfers data from the optimization executioner back and forth to the sub-apps that are running the actual Forward and adjoint FEM simulations.  Since no mesh or physics kernels are required on the main app, we use the [StochasticTools](syntax/StochasticTools/index.md) action to set up all of the nullkernels, empty mesh, etc. needed to get a MOOSE simulation to run.  

The [optimize](Optimize.md) executioner block, shown below, provides an interface with the [PETSc TAO](https://www.mcs.anl.gov/petsc/documentation/taosolvertable.html) optimization library. The optimization algorithm is selected with `tao_solver` with specific solver options set with `petsc_options`.


!listing test/tests/formfunction/objective_gradient_minimize/point_loads/master.i
         block=Executioner

The `Optimize` executioner requires a [FormFunction](syntax/FormFunction/index.md), shown below, to transfer data between the optimization executioner and the transfers used for communicating with the sub-apps.  The type of `FormFunction` depends on the optimization algorithm, given by teh  

!listing test/tests/formfunction/objective_gradient_minimize/point_loads/master.i
         block=FormFunction



The optimization executioner on the main app does not need a mesh or variables and uses the action to set-up a dummy mesh and variables.  

!media media/fig_forward.png
      style=width:60%;margin:auto;padding-top:2.5%;
      id=figForward
      caption=Optimization results for the (a) temperature field being matched at the $\times$ symbols in [figSetup] and (b) the intensity of the parameterized point loads applied at the $\bigcirc$ needed to produce the temperature field in (a)

!media media/fig_adjoint.png
      style=width:60%;margin:auto;padding-top:2.5%;
      id=figAdjoint
      caption=Adjoint temperature field and misfit load from first optimization iteration.

The first step is to define the simulation to perform, in this case the simulation considered is a 2D
steady state diffusion equation with Dirichlet boundary conditions on each end of the domain: find $u$
such that

!equation
\frac{\partial u}{\partial t} + \frac{\partial^2 u}{\partial x^2} = 0 \quad x \in [0,1],

where $u(0) = U(0, 0.5)$, $u(1) = U(1,2)$, and $U(a,b)$ defines a
[continuous uniform distribution](https://en.wikipedia.org/wiki/Uniform_distribution_%28continuous%29)
with $a$ and $b$ defining the lower and upper limits of the distribution, respectively.



## Sub-Application Input

The problem defined above, with respect to the [MultiApps] system, is a sub-application. The
complete input file for the problem is provided in [master-app]. The only item required
to enable the stochastic analysis is the [Controls] block, which contains a
[SamplerReceiver](/SamplerReceiver.md) object, the use of which will be explained
in the following section.

!listing test/tests/formfunction/objective_gradient_minimize/point_loads/master.i
         id=master_app
         caption=Complete input file for executing the transient diffusion problem.

!listing test/tests/formfunction/objective_gradient_minimize/point_loads/forward.i
        id=forward_app
        caption=Complete input file for executing the transient diffusion problem.

!listing test/tests/formfunction/objective_gradient_minimize/point_loads/adjoint.i
        id=adjoint_app
        caption=Complete input file for executing the transient diffusion problem.

## Master Input

The master application, with respect to the [MultiApps] system, is the driver of the stochastic
simulations, by itself it does not perform a solve. The complete input file for the master
application is shown in [monte-carlo-master], but the import sections will be detailed individually.

First, [Distributions] for each of the two stochastic boundary conditions are defined.

!listing test/tests/formfunction/objective_gradient_minimize/function_load/master.i block=FormFunction

Second, a [MonteCarloSampler](/MonteCarloSampler.md) is defined that utilizes the
two distributions and creates the Monte Carlo samples.

!listing test/tests/formfunction/objective_gradient_minimize/function_load/master.i block=FormFunction

Notice, that this sampler only executes on "initial", which means that the random numbers are
created once during the initial setup of the problem and not changed again during the simulation.

Next, a [SamplerTransientMultiApp](/SamplerTransientMultiApp.md) object is created. This object
creates and runs a sub-application for each sample provided by the sampler object.

!listing test/tests/formfunction/objective_gradient_minimize/function_load/master.i block=MultiApps

Finally, the [SamplerParameterTransfer](/SamplerParameterTransfer.md) is utilized to communicate the
sampler data to the sub-application. The 'parameters' input lists the parameters on the
sub-applications to perturb and the 'to_control' specifies the
[SamplerReceiver](/SamplerReceiver.md) object in the sub-application.

!listing test/tests/formfunction/objective_gradient_minimize/function_load/master.i block=FormFunction

!listing test/tests/formfunction/objective_gradient_minimize/function_load/master.i
         id=monte-carlo-master
         caption=Complete input file for master application for executing Monte Carlo stochastic
                 simulations.
