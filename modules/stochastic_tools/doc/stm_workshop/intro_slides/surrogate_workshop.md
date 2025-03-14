# Surrogate System

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

# [Trainers](Trainers/index.md)

!row!
!col! width=60%
- Trainer objects take in predictor and response data.
- Trainers need sampler to understand how data was parallelized.
- Predictor data usually comes from sampler values, but can be from other data.
- Most trainer objects have cross-validation capabilities to predict model performance.
- Trainer data can be stored in a binary output to be loaded later.
!col-end!

!col! width=40%

!listing! style=font-size:60%;width:500px
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
!listing-end!

!col-end!
!row-end!

!---

# [Surrogates](Surrogates/index.md)

!row!
!col! width=60%
- Trainer data is loaded in a Surrogate object
- Surrogate objects can be picked up by other objects and evaluated:

  !listing! language=cpp style=font-size:60%
  const SurrogateModel & model = getSurrogateModel("lm");
  std::vector<Real> x = {1, 2};
  Real y = model.evaluate(x);
  !listing-end!

!col-end!

!col! width=5%
!!
!col-end!

!col! width=35%

!listing! style=font-size:60%
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
!listing-end!
!col-end!
!row-end!

!---

# [Cross Validation](examples/cross_validation.md)

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

!listing! style=font-size:60%
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
!listing-end!
!col-end!
!row-end!

!---

# Workshop Surrogate Input

!row!
!col! width=50%

Instead of running the physics application, we load a previous CSV file with the necessary data.

!style fontsize=80%
- [CSVSampler](CSVSampler.md)
- [Cartesian1D](Cartesian1DSampler.md)
- [PolynomialRegressionTrainer](PolynomialRegressionTrainer.md)
- [PolynomialRegressionSurrogate](PolynomialRegressionSurrogate.md)
- [CrossValidationScores](CrossValidationScores.md)
- [EvaluateSurrogate](EvaluateSurrogate.md)

!style fontsize=60%
!listing examples/workshop/step06.i
         style=max-height:300px;max-width:500px

!col-end!

!col! width=50%

!plot scatter data=[{'x': [0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0],
                     'y': [279.54896097131353, 281.0305552737005, 282.39579342089087, 283.6502195222098, 284.7993776869822, 285.8488120245331, 286.8040666441874, 287.67068565526995, 288.4542131671062, 289.16019328902064, 289.79417013033867, 290.3616878003849, 290.8682904084846, 291.3195220639625, 291.7209268761438, 292.07804895435345, 292.3964324079165, 292.6816213461579, 292.9391598784025, 293.1745921139753, 293.39346216220144],
                     'name': 'Diffusivity'},
                    {'x': [0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0],
                     'y': [271.1521868168473, 272.6297412356422, 274.1048712559997, 275.5778283691649, 277.0488640663829, 278.5182298388991, 279.9861771779587, 281.45295757480665, 282.9188225206882, 284.3840235068486, 285.8488120245331, 287.3134395649867, 288.7781576194547, 290.24321767918235, 291.70887123541485, 293.1753697793973, 294.64296480237476, 296.1119077955927, 297.5824502502961, 299.0548436577303, 300.52933950914036],
                     'name': 'Source'},
                    {'x': [0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0],
                     'y': [198.8464566087304, 207.5534535524451, 216.25767668229207, 224.95960272829365, 233.65970842047267, 242.35847048885154, 251.05636566345277, 259.753870674299, 268.4514622514128, 277.1496171248167, 285.8488120245331, 294.5495236805849, 303.25222882299454, 311.95740418178445, 320.66552648697746, 329.37707246859577, 338.0925188566622, 346.8123423811994, 355.53701977222977, 364.26702775977566, 373.00284307386016],
                     'name': 'Left Fixed Temperature'},
                    {'x': [0.0, 0.04, 0.08, 0.12, 0.16, 0.2, 0.24, 0.28, 0.32, 0.36, 0.4, 0.44, 0.48, 0.52, 0.56, 0.6, 0.64, 0.68, 0.72, 0.76, 0.8, 0.84, 0.88, 0.92, 0.96, 1.0],
                     'y': [285.8488120245331, 286.7247767007411, 287.6004743868672, 288.4758889477085, 289.35100424806154, 290.22580415272336, 291.10027252649036, 291.97439323415944, 292.8481501405274, 293.7215271103909, 294.5945080085468, 295.4670766997918, 296.3392170489227, 297.2109129207359, 298.0821481800285, 298.9529066915975, 299.8231723202391, 300.6929289307503, 301.562160387928, 302.4308505565685, 303.29898330146904, 304.16654248742606, 305.0335119792363, 305.89987564169667, 306.765617339604, 307.63072093775486],
                     'name': 'Right Heat Flux'}]
              layout={'title': 'Surrogate Evaluation with Catesian1D',
                      'xaxis': {'title': 'Normalized Parameter'},
                      'yaxis': {'title': 'Average Temperature'},
                      'width': 450, 'height': 500,
                      'legend': {'x': 0.1},
                      'margin': {'l': 60, 't': 40, 'r': 10, 'b': 40}}

!col-end!

!row-end!
