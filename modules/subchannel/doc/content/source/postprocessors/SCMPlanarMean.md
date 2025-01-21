# SCMPlanarMean

!syntax description /Postprocessors/SCMPlanarMean

## Overview

<!-- -->

This is a postprocessor that calculates a mass-flow-rate-averaged mean value of a user specified [!param](/Postprocessors/SCMPlanarMean/variable) over all subchannels
at a user specified [!param](/Postprocessors/SCMPlanarMean/height).

## Example Input File Syntax

!listing /test/tests/problems/psbt/psbt.i block=Postprocessors language=cpp

!syntax parameters /Postprocessors/SCMPlanarMean

!syntax inputs /Postprocessors/SCMPlanarMean

!syntax children /Postprocessors/SCMPlanarMean
