# Deep Reinforcement Learning Control Using Libtorch

The following example showcases how to set up a Deep Reinforcement Learning (DRL)
training sequence for the generation of neural-net-based controllers of simulations
with MOOSE applications.

# Problem Statement

In this example we would like to design a DRL-based controller for the air conditioning of a
room. The room in this problem is a 2D box presented below:

The control problem can then be defined as follows: Try to ensure that the temperature at the
sensor position is as close to $297~K$ as possible. For this, we are allowed to use
the current and past values of the temperature at the sensor together with the current and past measurements
of the environment temperature. Furthermore, we assume the following:

- The density of the air is: $\rho = 1.184~\frac{kg}{m^3}$
- The specific heat of the air is: $\c_p = 1000 \frac{J}{kg~K}$
- The effective thermal condictivity (increased to account for mixing effects) is: $k = 0.5 \frac{W}{m~K}$
- The environment temperature is is handled as a Dirichlet boundary condition with a value of:
  $T_\mathrm{end}(t)~[K]=273+15*\sin{\left(\frac{\pi t}{86400}\right)}$
- The airconditioniner is modeled as a Neumann boundary condition on the top with given heatflux

## Input Files

For the training of a controller, we need two input files. A main input file which runs the
trainer and a another input file which sets up a sub-application that provides data for the
training procedure. We start by discussing the sub-application since it showcases the
physical problem at hand.

### Sub-application

The input file of the sub-application is requires the following essential changes in order for
the control problem. First, we need to add a Neumann boundary condition for the top surface:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=BCs

The `value` parameter of the boundary condition will be controlled by the neural network.
It is visible that the environment temperature is taken as a function of time. It is defined in the
`Functions` block:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=Functions

This block contains the function definition for the target temperature and the reward function as well.
For this experiment, we set the target temperature to be $297~K$, while the reward function
is defined in [DRLRewardFunction.md].

Next, we set up the data collection using postprocessors and reporters:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=Postprocessors

It is visible that we have two postprocessors measuring the temperature at the location of the sensor.
One saves the value at the beginning of the timestep, while the other saves it at the end of the
timestep. This is due to the fact that the neural network needs the measured temperature at the
beginning of the timestep, while we would like to compute the reward for the training process
using teh temperature at the end of the timestep. Furthermore, the trainer needs the environment temperature
and the reward values as well. Additionally, we save the action of the neural net together with its
logarithmic probability. Our control object will be responsible to populate these postprocessors.
Finally, we create an [AccumulateReporter.md] to store every data point throughout the simulation:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=Reporters

The last step is to set up the neural-net-based controller to the input file:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=Controls

For this, we need to supply the controllable parameters using [!param](/Controls/LibtorchDRLControl/parameters).
Then we supply the neural net inputs using [!param](/Controls/LibtorchDRLControl/responses).
Following this, the containers for the action and its logarithmic probability are attached using
[!param](/Controls/LibtorchDRLControl/action_postprocessors) and [!param](/Controls/LibtorchDRLControl/log_probability_postprocessors). LAstly we define the sacling of the input (responses) and the output
of the neural net. These must be consistent with the one defined within the [LibtorchDRLControlTrainer.md]
object in the main application.

The entire input file for the main application is below:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i

### Main Application

The input for the main application start with the definition of a `Sampler`. In this example we
do not aim to train a controller which can adapt to random model parameters, so the we just
define a dummy sampler which does not rely on random numbers.

!listing examples/libtorch_drl_control/libtorch_drl_control_traoner.i block=Samplers

In case we wpuld like to increase the
flexibility of the learner, this sampler can be switched to something that can scan the
uncertain input parameter space.

Following this, a `MultiApp` is created to run the subapplication many times for data generation:

!listing examples/libtorch_drl_control/libtorch_drl_control_traoner.i block=MultiApps

We also require transfers between the application. We need the pull the data of the
simulations from the `MultiApp` and we can use a [MultiAppReporterTransfer.md] to do it.

!listing examples/libtorch_drl_control/libtorch_drl_control_traoner.i block=Transfers

The neural network trained by [LibtorchDRLControlTrainer.md] needs to be transferred to the
control object in the sub-application. We can do this using [LibtorchNeuralNetControlTransfer.md].

Finally, we can set up our trainer object for the problem:

!listing examples/libtorch_drl_control/libtorch_drl_control_traoner.i block=Trainers



