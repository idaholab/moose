# PlanarMean

!syntax description /Postprocessors/PlanarMean

## Overview

<!-- -->

This is a postprocessor that calculates a mass-flow-rate-averaged mean value of a user specified [!param](/Postprocessors/PlanarMean/variable) over all subchannels
at a user specified [!param](/Postprocessors/PlanarMean/height).

## Example Input File Syntax

!listing /test/tests/problems/psbt/psbt.i block=Postprocessors language=cpp

!syntax parameters /Postprocessors/PlanarMean

!syntax inputs /Postprocessors/PlanarMean

!syntax children /Postprocessors/PlanarMean
