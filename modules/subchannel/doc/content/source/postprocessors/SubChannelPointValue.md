# SubChannelPointValue

!syntax description /Postprocessors/SubChannelPointValue

## Overview

<!-- -->

The user needs to specify the variable whose value they want to see, the index of the subchannel and the height.
The postprocessor will interpolate the value of that variable at that location.

## Example Input File Syntax

!listing /test/tests/problems/SFR/THORS/FFM-5B.i block=Postprocessors language=cpp

!syntax parameters /Postprocessors/SubChannelPointValue

!syntax inputs /Postprocessors/SubChannelPointValue

!syntax children /Postprocessors/SubChannelPointValue
