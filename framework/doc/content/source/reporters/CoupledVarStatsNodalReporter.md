# CoupledVarStatsNodalReporter

!syntax description /Reporters/CoupledVarStatsNodalReporter

## Overview

CoupledVarStatsNodalReporter produces the following statistics for a
variable: Maximum, Minium, Average, Total Nodes. The
[!param](/Reporters/CoupledVarStatsNodalReporter/base_name) can be used to prepend a
name to each reporter.



## Example Input File Syntax

!listing ele_nodal_reporters/nodal_reporter/coupledvarstats.i block=elem_stats
  indent=2 header=[Reporters] footer=[]

!syntax parameters /Reporters/CoupledVarStatsNodalReporter

!syntax inputs /Reporters/CoupledVarStatsNodalReporter

!syntax children /Reporters/CoupledVarStatsNodalReporter
