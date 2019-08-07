# MultiAppPostprocessorInterpolationTransfer

!syntax description /Transfers/MultiAppPostprocessorInterpolationTransfer

## Overview

Performs a transfer of a PostProcessor value from sub-applications to a field variable on the
master application using interpolation based on the location of each sub-application.

## Example Input File Syntax

The following input file snippet demonstrates the use of the
MultiAppPostprocessorInterpolationTransfer to interpolate the average value from two sub-applications
to a field variable in the master application.

!listing multiapp_postprocessor_interpolation_transfer/master.i block=Transfers

!syntax parameters /Transfers/MultiAppPostprocessorInterpolationTransfer

!syntax inputs /Transfers/MultiAppPostprocessorInterpolationTransfer

!syntax children /Transfers/MultiAppPostprocessorInterpolationTransfer
