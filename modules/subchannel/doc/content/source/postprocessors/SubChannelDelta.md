# SubChannelDelta

!syntax description /Postprocessors/SubChannelDelta

## Overview

<!-- -->

The user defines the variable which change they want to see along the whole assembly. The postprocessor calculates a
massflow rate weighted average at the inlet and outlet and then prints the absolute difference.

## Example Input File Syntax

!listing /examples/coupling/THM/subchannel.i block=Postprocessors language=cpp

!syntax parameters /Postprocessors/SubChannelDelta

!syntax inputs /Postprocessors/SubChannelDelta

!syntax children /Postprocessors/SubChannelDelta
