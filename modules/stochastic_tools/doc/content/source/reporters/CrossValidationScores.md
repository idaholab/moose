# CrossValidationScores

!syntax description /Reporters/CrossValidationScores

## Overview

This reporter outputs the results of the [examples/cross_validation.md] study for each [surrogate model](Surrogates/index.md) specified in [!param](/Reporters/CrossValidationScores/models). The output is a vector of vectors reporter value for each model, where the first index is the response (a single value unless creating a surrogate for a vector of data) and the second index is the study's trial index.

!syntax parameters /Reporters/CrossValidationScores

!syntax inputs /Reporters/CrossValidationScores

!syntax children /Reporters/CrossValidationScores
