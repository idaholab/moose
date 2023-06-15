# CoupledVarStatsElementReporter

!syntax description /Reporters/CoupledVarStatsElementReporter

## Overview

CoupledVarStatsElementReporter produces the following statistics for a
variable: Maximum, Minium, Average, Integral, Total Elements. The
[!param](/Reporters/CoupledVarStatsElementReporter/base_name) can be used to prepend a
name to each reporter.



## Example Input File Syntax

!listing ele_nodal_reporters/element_reporter/coupledvarstats.i block=elem_stats
  indent=2 header=[Reporters] footer=[]

!syntax parameters /Reporters/CoupledVarStatsElementReporter

!syntax inputs /Reporters/CoupledVarStatsElementReporter

!syntax children /Reporters/CoupledVarStatsElementReporter
