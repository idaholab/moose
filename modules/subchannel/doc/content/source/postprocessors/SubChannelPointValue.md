# SubChannelPointValue

!syntax description /Postprocessors/SubChannelPointValue

## Overview

<!-- -->

The user needs to specify the [!param](/Postprocessors/SubChannelPointValue/variable) whose value they want to see, the [!param](/Postprocessors/SubChannelPointValue/index) of the subchannel and the
[!param](/Postprocessors/SubChannelPointValue/height). The postprocessor will interpolate and print the value of that subchannel variable at that location.

## Example Input File Syntax

!listing /test/tests/problems/SFR/THORS/FFM-5B.i block=Postprocessors language=cpp

!syntax parameters /Postprocessors/SubChannelPointValue

!syntax inputs /Postprocessors/SubChannelPointValue

!syntax children /Postprocessors/SubChannelPointValue
