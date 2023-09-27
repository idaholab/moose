# Chorin method

The Chorin method involves splitting the Navier-Stokes equations into two separate steps: a prediction step for the velocity field and a correction step for the pressure field.

Prediction Step: In this step, a tentative velocity field is computed by solving the advection and diffusion terms of the Navier-Stokes equations without considering the current pressure gradient term and linearizing the advective term. This step essentially predicts the velocity field at the next time step.
This is implemented in the Navier Stokes module in the [INSChorinPredictor.md] kernel.

Pressure Correction Step: The predicted velocity field is then used to compute a provisional pressure field using a Poisson equation derived from the incompressibility constraint. The pressure correction is applied to remove any divergence from the predicted velocity field.
This is implemented in the Navier Stokes module in the [INSChorinCorrector.md] kernel.

Correction Step: The predicted velocity field is corrected by subtracting the gradient of the pressure field computed in the previous step. This ensures that the final velocity field is divergence-free.
This is implemented in the Navier Stokes module in the [INSChorinPressurePoisson.md] kernel.

Advantages to this scheme:

- Simplicity: Chorin's method is one of the easiest concepts of segregated solvers to implement and understand.
- Speed: segregated solvers solve smaller systems than fully-coupled solvers due to the segregation, and this can result in faster solves

Observed disadvantages:

- Accuracy: The accuracy can be limited compared to more advanced methods like fractional step methods or implicit methods.
- Pressure Oscillations: Chorin's method can sometimes lead to spurious pressure oscillations, especially in situations with strong gradients in the flow field.

!alert note
The Chorin method should be implemented using either multi-system, field-split or [MultiApps](syntax/MultiApps/index.md) capabilities.
There are currently no examples of this implementation in the module.
