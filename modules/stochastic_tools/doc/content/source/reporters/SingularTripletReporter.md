# SingularTripletReporter

!syntax description /Reporters/SingularTripletReporter

## Overview

This object is responsible for outputting singular triplets (left and right singular vectors together
with the corresponding singular values) from a [PODMapping.md]. The process is designed to work with
a JSON format. One can select the variables whose singular triplets shall be printed using
the [!param](/Reporters/SingularTripletReporter/variables) parameter.

## Example Input File Syntax

!listing test/tests/variablemappings/pod_mapping/pod_mapping_main.i block=Reporters

## Syntax

!syntax parameters /Reporters/SingularTripletReporter

!syntax inputs /Reporters/SingularTripletReporter

!syntax children /Reporters/SingularTripletReporter
