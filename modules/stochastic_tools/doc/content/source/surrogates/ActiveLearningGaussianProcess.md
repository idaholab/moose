# ActiveLearningGaussianProcess

!syntax description /Trainers/ActiveLearningGaussianProcess

## Description

The theory behind Gaussian Process (GP) is described in [GaussianProcessTrainer](GaussianProcessTrainer.md). `ActiveLearningGaussianProcess` is slightly similar to the `GaussianProcessTrainer` class in that it trains a GP model. However, a key feature of `ActiveLearningGaussianProcess` is that it permits re-training the GP model on-the-fly during the active learning process. This means that the input the inputs and output data set sizes will be dynamic and re-training the GP can be performed several times as dictated by the learning (or acquisition) function during active learning.

Just like the `GaussianProcessTrainer` class, the GP model during active learning can be trained using either of the following options:

- +PETSc/TAO+

  Relies on the TAO optimization library from PETSc. Several optimization algorithms are available from this library. Note that these algorithms perform deterministic optimization.

- +Adaptive moment estimation (Adam)+

  Relies on the pseudocode provided in [!cite](kingma2014adam). Adam permits stochastic optimization, wherein, a batch of the training data can be randomly chosen at each iteration.

## Interaction between `ActiveLearningMonteCarloSampler`, `ActiveLearningGaussianProcess`, and `ActiveLearningGPDecision`

!media ActiveLearning_consolidated.svg style=width:75%; id=al1_sch caption=Schematic of active learning in Monte Carlo simulations with parallel computing. The interaction between the three objects, `ActiveLearningMonteCarloSampler`, `ActiveLearningGaussianProcess`, and `ActiveLearningGPDecision`, is presented.

## Usage of active learning

Please refer to [ActiveLearningGPDecision](ActiveLearningGPDecision.md) on a detailed description on
using active learning.

!syntax parameters /Trainers/ActiveLearningGaussianProcess

!syntax inputs /Trainers/ActiveLearningGaussianProcess

!syntax children /Trainers/ActiveLearningGaussianProcess
