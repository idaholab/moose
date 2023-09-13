# BiFidelityActiveLearningGPDecision

!syntax description /Reporters/BiFidelityActiveLearningGPDecision

## Description

The `BiFidelityActiveLearningGPDecision` class has a very similar behavior as the [ActiveLearningGPDecision](ActiveLearningGPDecision.md) class. The figure below demonstrates the behavior of active learning with bi-fidelity modeling using Monte Carlo sampling with [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md). Instead of relying on a Gaussian Process (GP) prediction by default, a low-fidelity (LF) model prediction that is cheap to evaluate is used. Then, a GP correction is added to the LF model prediction to improve its quality and also quantify the LF prediction uncertainty. The GP itself is trained on the differences between the high-fidelity (HF) and LF model predictions. If the GP-corrected LF model prediction is not acceptable based on the uncertainty information, only then, the expensive HF model is called. Otherwise, the GP-corrected LF model predictions are used in a Monte Carlo sampler. Moreover, the calls to both the LF and HF model can be parallelized with the `num_batch` option in [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md). When set to 1, it represents a serial Monte Carlo sampling.

!media BFActiveLearning.png style=width:75%; id=bfal_sch caption=Schematic of the bi-fidelity active learning process in MOOSE.

## Input file syntax

The input file syntax is largely similar to GP-based active learning described in [ActiveLearningGPDecision](ActiveLearningGPDecision.md). There are three fundamental differences for leveraging bi-fidelity modeling.

First, the `MultiApps` block needs to have two subApps, one for the LF model and one for the HF model. This is shown in the listing below.

!listing modules/stochastic_tools/test/tests/reporters/BFActiveLearning/main_adam.i block=MultiApps

Second, the `Transfers` block needs to transfer the stochastic parameters to both the LF and HF models. Also, the outputs need to be transferred back to the mainApp from both the LF and HF models. This is shown in the listing below.

!listing modules/stochastic_tools/test/tests/reporters/BFActiveLearning/main_adam.i block=Transfers

Third, instead of relying on [ActiveLearningGPDecision](ActiveLearningGPDecision.md) for evaluating the quality of the GP-corrected LF model prediction, we rely on `BiFidelityActiveLearningGPDecision`. This takes into account the LF model predictions, as shown in the listing below.

!listing modules/stochastic_tools/test/tests/reporters/BFActiveLearning/main_adam.i block=Reporters

!syntax parameters /Reporters/BiFidelityActiveLearningGPDecision

!syntax inputs /Reporters/BiFidelityActiveLearningGPDecision

!syntax children /Reporters/BiFidelityActiveLearningGPDecision
