# Force Inversion Example: Point Loads

## Background

The MOOSE optimization module provides a flexible framework for solving inverse optimization problems in MOOSE.  This page is part of a set of examples for different types of inverse optimization problems.

- [Theory](theory/InvOptTheory.md)
- [Examples overview](optimization/examples/index.md)
- [Example 1: Point Loads](forceInv_pointLoads.md)
- [Example 2: Neumann Boundary Condition](forceInv_NeumannBC.md)
- [Example 3: Distributed Body Load](forceInv_BodyLoad.md)
- [Debugging Help](debuggingHelp.md)

# Example 1: Point Loads id=sec:pointLoad

In this example we are parameterizing the heat source intensity at the locations indicated by the $\bigcirc$ symbols in [figSetup] that will produce a temperature field that most closely matches the temperature measurements taken at the points indicated by the $\times$ symbols.  Dirichlet boundary conditions are applied to the entire boundary with T=300.

!media large_media/optimization/fig_pointLoadSetup.png
       style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
       id=figSetup
       caption=Meshed geometry with measurement ($\times$) and parameterized point load ($\bigcirc$) locations.

The temperature field with a known heat source is shown in [figForward].  This will be used as synthetic measurement data in the optimization example where the magnitude of the point sources ($\bigcirc$) will be parameterized to best match the temperatures at the measurement locations ($\times$).

!media large_media/optimization/fig_pointLoadSoln.png
      style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=figForward
      caption=Synthetic temperature field for known heat sources for optimization example.

## Main Application Input

Optimization problems are solved using the [MultiApps](MultiApps/index.md) system.  The main application contains the optimization executioner and the sub-applications solve the forward and adjoint PDE.   The main application input is shown in [fig:main_app].

!listing test/tests/optimizationreporter/point_loads/main.i
         id=fig:main_app
         caption=Main application optimization input for point load parameterization shown in [figSetup]

The main application runs the optimization executioner and transfers data from the optimization executioner back and forth to the sub-apps that are running the "forward" and "adjoint" simulations.

!alert note
Since no mesh or physics kernels are required on the main-app, we use the [Optimization](Optimization/index.md) action to set up all of the nullkernels, empty mesh, etc. needed to get a MOOSE simulation to run.

The [Optimize](Optimize.md) executioner block in [executionerBlock] , provides an interface with the [PETSc TAO](https://www.mcs.anl.gov/petsc/documentation/taosolvertable.html) optimization library. The optimization algorithm is selected with [!param](/Executioner/Optimize/tao_solver) with specific solver options set with `petsc_options`.  In this example the TAO Hessian based Newton linesearch algorithm is selected with `taonls`.

!listing test/tests/optimizationreporter/point_loads/main.i
         block=Executioner id=executionerBlock
         caption=Main application `Executioner` block for point load parameterization shown in [figSetup]

The [optimize](Optimize.md) executioner requires an [OptimizationReporter](syntax/OptimizationReporter/index.md), shown below, to transfer data between the optimization executioner and the transfers used for communicating with the sub-apps.  [!param](/OptimizationReporter/OptimizationReporter/parameter_names) are the list of parameters being controlled.  The [!param](/OptimizationReporter/OptimizationReporter/num_values) specifies the number of parameters per group of `parameter_names` being controlled.  In this case there is a single parameter being controlled and that parameter contains three values being controlled.  These three values are the magnitude of the point loads being applied, shown by the $\bigcirc$ symbols in [figSetup].  [!param](/OptimizationReporter/OptimizationReporter/initial_condition) can be used to supply initial guesses for parameter values.  Bounded optimization algorithms like bounded conjugate gradient (`taobncg`) will use the bounds supplied by [!param](/OptimizationReporter/OptimizationReporter/lower_bounds) and [!param](/OptimizationReporter/OptimizationReporter/upper_bounds).  Unbounded solvers like `taonls` used in this example will not apply the bounds even if they are in the input file.

The [!param](/OptimizationReporter/OptimizationReporter/measurement_points) are the xyz coordinates of the measurement data and [!param](/OptimizationReporter/OptimizationReporter/measurement_values) are the values at each measurement point.  For this small example, the measurement data are supplied in the input file but for larger sets of measurement data, there is the option to read these values from a csv file using [!param](/OptimizationReporter/OptimizationReporter/measurement_file) and [!param](/OptimizationReporter/OptimizationReporter/file_value).

!listing test/tests/optimizationreporter/point_loads/main.i
         block=OptimizationReporter id=optimizationReporterBlock
         caption=Main application `OptimizationReporter` block for point load parameterization shown in [figSetup]

The `MultiApps` block is shown in [multiapps].  The optimization executioner utilizes [FullSolveMultiApp](multiapps/FullSolveMultiApp.md) sub-apps which, when the module is linked in, have special [!param](/MultiApps/FullSolveMultiApp/execute_on) flags to allow the optimization executioner to control which sub-app is being solved.  This allows TAO to call whichever sub-app it needs in order to compute the quantity needed.  For instance, if TAO needs to recompute the objective function with a new set of parameters, it will execute the sub-app with the flag `exectute_on=FORWARD`.  The sub-apps being set-up correspond to the type of optimization algorithm being used.  Gradient free algorithms like Nelder-Mead (`taonm`) only require a forward sub-app.  Gradient based optimization algorithms like conjugate gradient (`taobncg`) require a forward and adjoint sub-app.  Hessian based optimization algorithms like Newton linesearch (`taonls`) requires a forward, adjoint and homogeneous forward sub-app.

!listing test/tests/optimizationreporter/point_loads/main.i
          block=MultiApps id=multiapps
          caption=Main application `MultiApps` block for point load parameterization shown in [figSetup]

`Transfers` makes up the next section of the main input file.  TAO interacts with the various sub-apps using the reporter system and the [MultiAppReporterTransfer.md].  The first three `Transfers` in [transfers] communicate data with the forward sub-app.  These `Transfers` are used by TAO to compute the objective. The first transfer `[toForward_measument]` communicates the measurement locations to the forward app object [OptimizationData.md].  These are the locations where the objective function is computed by minimizing the difference between the simulated and experimental data at discrete points, see [!eqref](theory/InvOptTheory.md#eqn:objective_integral).  The second transfer `[toForward]` is used by TAO to control the parameter being optimized.  In this case, `[toForward]` controls the 'value' reporter in a [ConstantVectorPostprocessor.md] on the forward-app which is then consumed by the [ReporterPointSource.md] dirac kernel to apply the point source loading.  The third transfer, `[fromForward]` returns the simulation values at the measurement points from the forward-app to the main-app `[OptimizationReporter]` which computes a new objective function for TAO.

!listing test/tests/optimizationreporter/point_loads/main.i
          block=Transfers
          id=transfers
          caption=Main application `Transfers` for point load parameterization shown in [figSetup]

The next set of `Transfers` in [transfers] communicate reporter values between the main-app and adjoint sub-app to compute the gradient of the objective function with respect to the controllable parameter.  The fourth transfer named `[toAdjoint]` sends the misfit data at each measurement point to an [OptimizationData.md] reporter on the adjoint-app which is then consumed by the [ReporterPointSource.md] dirackernel to apply the misfit source loading as described by the top equation in [!eqref](theory/InvOptTheory.md#eqn:adjoint_problem).  The fifth transfer named `[fromAdjoint]` retrieves the gradient from the adjoint-app for TAO.

The final set of `Transfers` in [transfers] communicate reporter values between the main-app and the homogeneous forward sub-app to compute a matrix free Hessian.  The homogeneous forward transfers, `[toHomoForward_measument]` `[toHomoForward]` and `[fromHomoForward]` transfer the same reporter data to the same objects types used by the forward-app.  For force inversion, the homogeneous forward app solves the same kernels as the forward-app except the boundary conditions are homogenized, i.e. set to zero.  The parameter being controlled in [OptimizationReporter](syntax/OptimizationReporter/index.md) by the [Optimize.md] executioner for the homogeneous forward problem is the variation in the parameter which is determined by TAO in order to build up the required matrix free Hessian.

Iteration history of the optimization solve is output by the [OptimizationInfo.md] reporter as shown in [optinfo].  This reporter tracks the objective function value, norm of the gradient, total number of sub-app evaluations, and the number of forward, adjoint, or Hessian iterations per optimization step.

!listing test/tests/optimizationreporter/point_loads/main.i
          block=Reporters
          id=optinfo
          caption=Main application [OptimizationInfo.md] for outputting convergence information.

## Forward Sub-Application Input

The forward problem sub-app input file for the point load simulation shown in [figSetup] is given in [forward_app].  Tao uses the solution of the forward-app to compute the objective function.  Almost every object included in the forward-app input would be needed in a regular simulation of this system with point loads of a known magnitude.  The point loads are applied by the [ReporterPointSource.md] Dirac kernel.  The xyz coordinates and point load value used in the `ReporterPointSource` come from the reporters defined in the [ConstantVectorPostprocessor.md] vector postprocessor.  The reporters in the `ConstantVectorPostprocessor` are over-written by the `[toForward]` transfer on the main-app each time Tao executes the forward-app.  The values being transferred into the `ConstantVectorPostprocessor` are the values being optimized and are controlled by Tao.

!listing test/tests/optimizationreporter/point_loads/forward.i
         id=forward_app
         caption=Complete input file for executing the forward problem sub-app.

The locations where the simulation values are compared to the measurement data is transferred from the main-app `[toForward_measurement]` transfer into the forward-app [OptimizationData.md] reporter.  By specifying the [!param](/Reporters/OptimizationData/variable), the reporter will evaluate the simulated temperature at the measurement locations.  The main-app `[fromForward]` transfer then gets the temperature values sampled by `OptimizationData` back to Tao and the objective function can then be computed.

## Adjoint Sub-Application Input

The adjoint problem sub-app computes the adjoint solution of the forward problem for a point load applied at the measurement locations.  The adjoint sub-app input file is given in [adjoint_app]. The adjoint variable is used to compute the gradient needed by TAO.  The magnitude of the point loads for the adjoint problem are the misfit given by the difference of the measurement and simulation data, as shown by [!eqref](theory/InvOptTheory.md#eqn:adjoint_problem).  The misfit is computed by the [OptimizationReporter.md] on the main-app and is transferred into the adjoint-app `[OptimizationData]` object using the main-app `[toAdjoint]` transfer.  The [ReporterPointSource.md] consumes the `OptimizationData` and applies the point loads at the measurement points.

!listing test/tests/optimizationreporter/point_loads/adjoint.i
         id=adjoint_app
         caption=Complete input file for executing the adjoint problem sub-app.

The misfit load applied at the first iteration step is given by the residual values shown in [figAdjointRes].  Homogeneous boundary conditions are applied in the adjoint problem.  The solution to the adjoint problem for the first iteration is shown in [figAdjoint].  The adjoint variable can be interpreted as the sensitivity of point load locations on the measurement values. Ideally, measurements would be taken at locations that are most sensitive to the point source locations, i.e. the measurement locations maximize the adjoint variable at the point sources.

!media large_media/optimization/fig_pointLoadResidual.png
      style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=figAdjointRes
      caption=Adjoint misfit load from the first optimization iteration.

!media large_media/optimization/fig_pointLoadAdjoint.png
      style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=figAdjoint
      caption=Adjoint solution from the misfit load applied in [figAdjointRes] for the first optimization iteration.

The purpose of the adjoint sub-app is to compute gradient needed by TAO.  This gradient is given by

\begin{equation}
\frac{\mathrm{d}f}{\mathrm{d}\mathbf{p}} = \mathbf{\lambda}\frac{\partial\mathcal{R}}{\partial\mathbf{p}}.
\end{equation}

where $f$ is the objective function, $p$, is the controllable parameter, $\lambda$ is the adjoint variable computed by the adjoint sub-app and $\mathcal{R}$ is the residual from the forward problem.  This gradient ignores regularization shown in [!eqref](theory/InvOptTheory.md#eq:objective) which isn't needed for force inversion problems.  For the simple case of point loading, the derivative of the forward problem residual with respect to the parameter, $\partial\mathcal{R}/\partial\mathbf{p}$, given by [!eqref](theory/InvOptTheory.md#eq:pointLoad) is a projection matrix equal to 1 at locations where measurements are taken and zero everywhere else.  The gradient given by $\lambda\cdot\partial\mathcal{R}/\partial\mathbf{p}$ is the adjoint variable sampled at the locations of the point loads. This sampling is done by the [PointValueSampler.md] vector postprocessor.  The `PointValueSampler` is then transferred back to main-app in the `fromAdjoint` transfer and is the gradient used by TAO.

## Homogeneous Forward Sub-Application Input

Hessian based optimization algorithms like Newton linesearch (`taonls`) require a homogeneous version of the forward problem given by the input file [homoForward_app].  A homogenous forward problem is the forward problem with homogenous boundary conditions, i.e. the value or the flux is to zero.  A homogenous forward problem is required for Hessian based optimization if a homogenous forward problem cannot be derived, then a gradient based algorithm like conjugate gradient (`taobncg`) should be used.  The forward and homogeneous forward sub-apps have the same parameters transferred to them from the main-app.  The parameters transferred into the homogeneous forward problem [ConstantVectorPostprocessor.md] vector postprocessor from the main-app `[toHomoForward]` transfer are perturbations of the parameters computed by TAO.  The solution to the homogeneous forward problem is then sampled at the measurement points by [OptimizationData.md] and is returned to TAO on the main-app with the `[fromHomoForward]` transfer to be used as a matrix free Hessian.  The matrix free Hessian algorithm is only valid for force inversion and does not work for material inversion.  For linear optimization problems, Hessian based optimization algorithms are able to converge on the exact answer in a single iteration.

!listing test/tests/optimizationreporter/point_loads/forward_homogeneous.i
    id=homoForward_app
    caption=Complete input file for executing the homogeneous forward problem sub-app.

## Optimization Results

The results for this example are shown in [figConvergence].  The left plot in [figConvergence] shows the temperature along a vertical line passing through the measurement points is taken at several conjugate gradient iterations.  The right plot shows the convergence of the objective function for several optimization algorithms.  It's important to note when looking at the convergence plot that the Nelder Mead gradient free algorithm requires only a single forward solve per iteration where as the gradient based algorithms require at least two solves, one for the forward problem and one for the adjoint problem.  The Hessian based algorithms require several forward, adjoint, and homogenous forward solves per iteration to make the matrix free Hessian accurate enough to converge.  Even with this in mind, the Hessian based algorithm is the most computationally efficient for this problem.  The `taolmvm` gradient based method also performs well and only requires an adjoint.

!media large_media/optimization/fig_pointLoadConvergence.png
      style=width:100%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=figConvergence
      caption=(Left) Temperature field taken along pink dashed line in [figSetup] at conjugate gradient iterations.  (Right) Convergence of the objective function for different optimization algorithms.
