!if! function=hasLibtorch()
# Deep Reinforcement Learning Control Using Libtorch

The following example demonstrates how to set up a Proximal Policy Optimization (PPO) based 
Deep Reinforcement Learning (DRL) training sequence for neural-net-based controllers of MOOSE simulations. 
See [!cite](schulman2017proximal) for a more theoretical background on the PPO algorithm.

## Problem Statement

In this example we would like to design a DRL-based controller for the air conditioning of a
room. The room in this problem is a 2D box shown in [!ref](problem_setup):

!media problem_statement.png style=display:block;margin-left:auto;margin-right:auto;width:40%;
       id=problem_setup caption=Problem setup for the DRL control example.

The control problem can then be defined as follows: control the heat flux at the top of the box such that
the temperature at the sensor position is as close to $297~K$ as possible. For this, we are allowed to use
the current and past values of the temperature at the sensor together with the current and past measurements
of the environment temperature. Furthermore, we assume the following:

- The density of the air is: $\rho = 1.184~\frac{kg}{m^3}$
- The specific heat of the air is: $c_p = 1000 \frac{J}{kg~K}$
- The effective thermal condictivity (increased to account for mixing effects) is: $k = 0.5 \frac{W}{m~K}$
- The environment temperature is handled as a Dirichlet boundary condition applied 
  to the right and left walls with a value of:
  $T_\mathrm{env}(t)~[K]=273+15*\sin{\left(\frac{\pi t}{86400}\right)}$
- The air conditioner is modeled as a Neumann boundary condition on the top with given heat flux

## Input Files

For training a DRL-based controller, we need two input files: a main input file which runs the
trainer and another input file which sets up a sub-application that provides data for the
training procedure. We start by discussing the sub-application since it showcases the
physical problem at hand.

### Sub-application

The sub-application input file requires the following
parts in order to enable the control functionalities discussed later. First, we need to add
a Neumann boundary condition for the top surface:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=BCs

The [!param](/BCs/NeumannBC/value) parameter of the boundary condition will
be controlled by the neural network. The environment temperature (`dirichlet`)
is defined to be a function of time (`temp_env`), which is specified in the `Functions` block:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=Functions

This block also contains the function definition for the target temperature and the reward function.
For this experiment, we set the target temperature (`design_function`) to be $297~K$, while the reward function
(`reward_function`), defined by [ScaledAbsDifferenceDRLRewardFunction.md], is used for the training.

Next, we set up the data collection using the `Postprocessors` and `Reporters` blocks:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=Postprocessors

Two of these postprocessors measure the temperature at the location of the sensor.
`center_temp` stores the value at the beginning of the time step, while `center_temp_tend` stores it at the end of the
time step. This is due to the fact that the neural network needs the measured temperature at the
beginning of the time step, while we would like to compute the reward for the training process
using the temperature at the end of the time step. Furthermore, the trainer needs the environment temperature
and the reward values as well. Additionally, we save the action (`top_flux`) of the neural net together with its
logarithmic probability (`log_prob_top_flux`). Our control object will be responsible to populate these two postprocessors.
Finally, we create an [AccumulateReporter.md] to store every data point throughout the simulation:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=Reporters

The last step is to set up the neural-net-based controller for the input file:

!listing examples/libtorch_drl_control/libtorch_drl_control_sub.i block=Controls

For this, we need to supply the controllable parameters using `parameters`.
Then, we supply the neural net inputs using `responses`.
Lastly we define the scaling of the input (responses) and the output
of the neural net. These must be consistent with the values of the [LibtorchDRLControlTrainer.md]
object in the main application.
The containers for the control signal and its logarithmic probability are defined in the `Postprocessors` block
using [LibtorchControlValuePostprocessor](source/libtorch/postprocessors/LibtorchControlValuePostprocessor.md) and
[LibtorchDRLLogProbabilityPostprocessor](source/libtorch/postprocessors/LibtorchDRLLogProbabilityPostprocessor.md) as
shown above.
Furthermore, the additional [LibtorchNeuralNetControl](LibtorchNeuralNetControl.md) (`src_control_final`)
can be used to evaluate the neural network without the additional random
sampling process needed for the training process. In other words, this object will evaluate the
final product of this training process.

### Main Application

The input for the main application starts with the definition of a [Sampler](CartesianProductSampler.md). In this example we
do not aim to train a controller which can adapt to random model parameters, so we just
define a dummy sampler which does not rely on random numbers.

!listing examples/libtorch_drl_control/libtorch_drl_control_trainer.i block=Samplers

In case we would like to increase the flexibility of the neural-net based controller,
this sampler can be switched to something that can scan the uncertain input parameter space.

Following this, a [MultiApp](SamplerFullSolveMultiApp.md) is created to run the sub-application many times for data generation:

!listing examples/libtorch_drl_control/libtorch_drl_control_trainer.i block=MultiApps

We also require transfers between the applications. We pull the data of the
simulations from the `MultiApp` using [MultiAppReporterTransfer.md].

!listing examples/libtorch_drl_control/libtorch_drl_control_trainer.i block=Transfers

The neural network trained by [LibtorchDRLControlTrainer.md] needs to be transferred to the
control object in the sub-application. We do this using [LibtorchNeuralNetControlTransfer.md].
Finally, we can set up our trainer object for the problem:

!listing examples/libtorch_drl_control/libtorch_drl_control_trainer.i block=Trainers

The trainer object will need the names of the reporters containing the responses (input of the neural net)
of the system together with the control signals, control signal logarithmic probabilities, and the rewards.
When these are set, we define the architecture of the critic and control neural nets
(see [!cite](schulman2017proximal) for more information on these) using
[!param](/Trainers/LibtorchDRLControlTrainer/num_critic_neurons_per_layer) and
[!param](/Trainers/LibtorchDRLControlTrainer/num_control_neurons_per_layer).
The corresponding learning rates can be defined by 
[!param](/Trainers/LibtorchDRLControlTrainer/critic_learning_rate) and 
[!param](/Trainers/LibtorchDRLControlTrainer/control_learning_rate).
Then, we copy-paste the input/output standardization options from the `Control` in the sub-app.
Additionally, we can choose to standardize the advantage function which makes
convergence more robust in certain scenarios.

Lastly, we request 1,000 epochs for training the neural networks in each iteration and
collect data from 10 simulations on the sub-app every time step of the main app.
We set the iteration number by setting the number of time steps below:

!listing examples/libtorch_drl_control/libtorch_drl_control_trainer.i block=Executioner

Which means that we run a total of 440 simulations.

## Results

First, we take a look at how the average episodic reward evolves throughout the
training process. We see that the average reward increases to a point where it is
comparable with the theoretical maximum.  

!alert note
In this example we used [ScaledAbsDifferenceDRLRewardFunction.md] as follows:
$R = 10 - |T_\mathrm{target} - T_\mathrm{sensor}|$ which means that the maximum achievable 
reward was 10. 

However, it is important to note
that the higher the standard deviation is for the controllers signal to the system 
(also referred to as action), the more flexible the controller
will be (due to less overfitting). On the other hand, by increasing the random
variation in the action, we also decrease its accuracy. Therefore, the user needs
to balance these two factors by tuning the parameters in the `Trainer` and `Control` objects.

!plot scatter
  id=reward caption=The evolution of the reward values over the iteration.
  filename=examples/libtorch_drl_control/gold/reward.csv
  data=[{'x':'time', 'y':'reward/average_reward', 'name':'Reward'},
        {'x':'time', 'y':'maximum', 'name':'Theoretical Maximum'}]
  layout={'xaxis':{'type':'linear', 'title':'Number of simulations'},
          'yaxis':{'type':'linear','title':'Average Episodic Reward'}}

Following the training procedure, we can replace the [LibtorchDRLControl.md] object with
[LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md)
to evaluate the final version of the neural network
without the additional randomization. By doing this, the following results are obtained:

!plot scatter
  id=results caption=The evolution of the room temperature at the sensor over the day.
  filename=examples/libtorch_drl_control/gold/results.csv
  data=[{'x':'time', 'y':'controlled', 'name':'Controlled'},
        {'x':'time', 'y':'uncontrolled', 'name':'Uncontrolled'},
        {'x':'time', 'y':'env', 'name':'Environment'},
        {'x':'time', 'y':'target', 'name':'Target'}]
  layout={'xaxis':{'type':'linear', 'title':'Time (s)'},
          'yaxis':{'type':'linear','title':'Temperature (K)'}}

We see that the controller is able to maintain the comfortable room temperature with a constantly changing
environment temperature.

!if-end!
