# SurrogateModelOutput

!syntax description /Outputs/SurrogateModelOutput

## Overview

[Surrogate models](Surrogates/index.md), once trained, can be output to a binary file for later
use by using the SurrogateModelOutput object. All data within the model that was delcared using
`declareModelData` is automatically stored in the generated file.

## Example Input File Syntax

The following snippet includes a surrogate model that is being trained and after the training is
complete the model data is output using the SurrogateModelOutput object.

!listing load_store/train.i block=Surrogates Outputs

!syntax parameters /Outputs/SurrogateModelOutput

!syntax inputs /Outputs/SurrogateModelOutput

!syntax children /Outputs/SurrogateModelOutput
