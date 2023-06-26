# NodalVariableStatistics

!syntax description /Reporters/NodalVariableStatistics

## Overview

NodalVariableStatistics produces the following statistics for a
variable: Maximum, Minium, Average, Total Nodes. The
[!param](/Reporters/NodalVariableStatistics/base_name) can be used to prepend a
name to each reporter.



## Example Input File Syntax

!listing nodal_reporter/nodal_stats.i block=nodal_stats
  indent=2 header=[Reporters] footer=[]

!syntax parameters /Reporters/NodalVariableStatistics

!syntax inputs /Reporters/NodalVariableStatistics

!syntax children /Reporters/NodalVariableStatistics
