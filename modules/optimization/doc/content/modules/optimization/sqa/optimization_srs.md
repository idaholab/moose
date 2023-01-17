!template load file=sqa/module_srs.md.template app=Optimization category=optimization

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the MOOSE Optimization module optimize model parameters such that it minimizes a defined objective. An example would be to search for material property values so that the solution matches experimental results. As such, the purpose of this module is not to provide physical model capabilities, which is typically the responsibility of other MOOSE modules and dependent applications.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The MOOSE Optimization module utilizes several MOOSE systems to perform optimization on physics models. The design relies on a customized Executioner to drive the optimization algorithm. This Executioner calls a specific type of Reporter known as a OptimizationReporter, which defines the parameter space, objective, gradient, and Hessian functions necessary for the optimization methods. Along with calling this Reporter, the Executioner calls MultiApps and Transfers that are responsible for passing parameters to the physics model, running the model, and gathering the necessary data to compute the objective, gradient, and Hessian. Typically, one sub-application defines the underlying physics for the objective computation, another defines an adjoint version of the model for the gradient computation, and another defines a homogeneous version of the model for the matrix-free Hessian computation. All data from these applications is gathered in the OptimizationReporter, which is then sent to the optimization Executioner. The resulting output is the value of the optimized parameters and the solution of physics model with these parameters. The scope of the Optimization module includes, but is not limited to:

- Inverse optimization
- Shape optimization
- Topology optimization

!! system-scope-finish
!template-end!
