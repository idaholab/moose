# SCMPlanarMean

!syntax description /Postprocessors/SCMPlanarMean

## Overview

!! Intentional comment to provide extra spacing

This is a postprocessor that calculates a mass-flow-rate-averaged mean value of a user specified [!param](/Postprocessors/SCMPlanarMean/variable) over all subchannels
at a user specified [!param](/Postprocessors/SCMPlanarMean/height).

## Example Input File Syntax

!listing /test/tests/problems/Lead-LBE-19pin/test_LBE-19pin.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SCMPlanarMean

!syntax inputs /Postprocessors/SCMPlanarMean

!syntax children /Postprocessors/SCMPlanarMean
