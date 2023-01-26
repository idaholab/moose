# ActiveLearningMonteCarloSampler

!syntax description /Samplers/ActiveLearningMonteCarloSampler

## Description

The `ActiveLearningMonteCarloSampler` facilitates the following tasks:

- +Propose random samples from distributions+

  For each transient step, `ActiveLearningMonteCarloSampler`, by default, proposes a user-specified batch of sample inputs using the objects defined in the `Distributions` block.

- +Bookkeeping samples whose surrogate output is poor+
 
  With the set of new input samples proposed, a trained surrogate model predicts the model outputs and also assess the quality of its own outputs. This prediction and quality assessment is done in the `ActiveLearningGPDecision` Reporter class. `ActiveLearningMonteCarloSampler` takes information from this Reporter class and keeps a record of the input samples for which the surrogate predictions are poor.

- +Launch batch of model evaluations in parallel+

  Once the number of input samples for which the surrogate predictions are poor equals or exceeds a user specified limit, as indicated by the parameter `num_batch`, the corresponding full model evaluations to these input samples are launched through the `SamplerFullSolveMultiApp`. The advantage of launching these models in a batch mode is that parallel computing can be efficiently utilized. Therefore, this workflow permits what is called as "parallelized active learning."

## Interaction between `ActiveLearningMonteCarloSampler`, `ActiveLearningGaussianProcess`, and `ActiveLearningGPDecision`

!media ActiveLearning_consolidated.svg style=width:75%; id=al1_sch caption=Schematic of active learning in Monte Carlo simulations with parallel computing. The interaction between the three objects, `ActiveLearningMonteCarloSampler`, `ActiveLearningGaussianProcess`, and `ActiveLearningGPDecision`, is presented.

## Usage of active learning

Please refer to [ActiveLearningGPDecision](ActiveLearningGPDecision.md) on a detailed description on
using active learning.

!syntax parameters /Samplers/ActiveLearningMonteCarloSampler

!syntax inputs /Samplers/ActiveLearningMonteCarloSampler

!syntax children /Samplers/ActiveLearningMonteCarloSampler