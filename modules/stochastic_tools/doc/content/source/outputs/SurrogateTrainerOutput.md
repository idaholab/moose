# SurrogateTrainerOutput

!syntax description /Outputs/SurrogateTrainerOutput

## Overview

[Surrogate models](Surrogates/index.md), once trained, can be output to a binary file for later
use by using the SurrogateTrainerOutput object. All data within the model that was delcared using
`declareTrainerData` is automatically stored in the generated file.

## Example Input File Syntax

The following snippet includes a surrogate model that is being trained and after the training is
complete the model data is output using the SurrogateTrainerOutput object.

!listing load_store/train.i block=Trainers Outputs

!syntax parameters /Outputs/SurrogateTrainerOutput

!syntax inputs /Outputs/SurrogateTrainerOutput

!syntax children /Outputs/SurrogateTrainerOutput
