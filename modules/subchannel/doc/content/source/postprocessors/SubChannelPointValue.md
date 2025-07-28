# SubChannelPointValue

!syntax description /Postprocessors/SubChannelPointValue

## Overview

!! Intentional comment to provide extra spacing

The user needs to specify the [!param](/Postprocessors/SubChannelPointValue/variable) whose value they want to see, the [!param](/Postprocessors/SubChannelPointValue/index) of the subchannel and the
[!param](/Postprocessors/SubChannelPointValue/height). The postprocessor will interpolate and print the value of that subchannel variable at that location.

## Example Input File Syntax

!listing /test/tests/problems/psbt/psbt_explicit.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SubChannelPointValue

!syntax inputs /Postprocessors/SubChannelPointValue

!syntax children /Postprocessors/SubChannelPointValue
