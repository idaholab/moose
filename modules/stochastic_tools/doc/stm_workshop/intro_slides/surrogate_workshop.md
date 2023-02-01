# Surrogate System

!style halign=center
[https://mooseframework.inl.gov/syntax/Trainers/index.html](https://mooseframework.inl.gov/syntax/Trainers/index.html)

!style halign=center
[https://mooseframework.inl.gov/syntax/Surrogates/index.html](https://mooseframework.inl.gov/syntax/Surrogates/index.html)

!---

# Introduction

- STM Trainers/Surrogates provide a way of building reduced-order models that emulate high-fidelity multiphysics models.
- These surrogates are meant to be much faster to evaluate than the high-fidelity model.
- Usages for these surrogates:

  - Uncertainty quantification
  - Parameter optimization
  - Multi-scale modeling
  - Controller design
  - Digital twins

- Process:

  1. Produce response data using sampling/multiapp strategy.
  1. Use Trainer object to compute meta data from predictor and response data.
  1. Load trainer data to create Surrogate object.
  1. Pick up Surrogate object to evaluate at inputted predictor values.

!---

# Trainers

!row!
!col! width=60%
- Trainer objects take in predictor and response data.
- Trainers need sampler to understand how data was parallelized.
- Predictor data usually comes from sampler values, but can be from other data.
- Most trainer objects have cross-validation capabilities to predict model performance.
- Trainer data can be stored in a binary output to be loaded later.
!col-end!

!col! width=40%
```
[Trainers]
  [linear_regression]
    type = PolynomialRegressionTrainer
    sampler = sample_train
    response = 'storage/data:avg_u:value'
    max_degree = 1
    regression_type = ols

    # Other options
    # predictors = ''
    # predictor_cols = ''
    # response_type = real/vector_real
  []
[]

[Outputs]
  [lm]
    type = SurrogateTrainerOutput
    trainers = linear_regression
  []
[]
```
!col-end!
!row-end!

!---

# Surrogates

!row!
!col! width=60%
- Trainer data is loaded in a Surrogate object
- Surrogate objects can be picked up by other objects and evaluated:

  ```C++
  const SurrogateModel & model = getSurrogateModel("lm");
  std::vector<Real> x = {1, 2};
  Real y = model.evaluate(x);
  ```
!col-end!

!col! width=5%
!!
!col-end!

!col! width=35%
```
[Surrogates]
  [lm]
    type = PolynomialRegressionSurrogate
    trainer = linear_regression
    # filename = 'lm_linear_regression.rd'
  []
[]

[Reporters]
  [lm_evaluate]
    type = EvaluateSurrogate
    model = lm
    sampler = sample_test
  []
[]
```
!col-end!
!row-end!

!---

# Cross Validation

!row!
!col! width=60%
- Cross validation is a method to determine the quality of surrogate.
- The method involves splitting the predictor/response data into train and test parts.

  - Randomly select a portion of the data to train with.
  - Test the model with rest of the predictor data and compute error with the response data
  - Repeat with "folds" of randomly selected train-test parts.

- The error is an indication of how the resulting model (trained with the full data set) fits the data

  - Large cross-validation scores indicates under-fitting.
  - Large variation in cross-validation scores indicates over-fitting.
!col-end!

!col! width=35%
```
[Trainers]
  [trainer]
    ...
    cv_type = k_fold
    cv_splits = 5
    cv_n_trials = 100
    cv_surrogate = surrogate
  []
[]

[Surrogates]
  [surrogate]
    ...
    trainer = trainer
  []
[]

[Reporters]
  [cv_scores]
    type = CrossValidationScores
    models = surrogate
  []
[]

```
!col-end!
!row-end!
