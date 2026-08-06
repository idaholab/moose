# SCMBulkOutletTemperature

!syntax description /Postprocessors/SCMBulkOutletTemperature

## Overview

This postprocessor computes the bulk outlet enthalpy using the outlet mass-flow
rates and specific enthalpies, then converts that state back to temperature with
the selected single-phase fluid-property model:

\begin{equation}
h_\mathrm{bulk,out} =
\frac{\sum_i \dot{m}_i h_i}{\sum_i \dot{m}_i},
\qquad
T_\mathrm{bulk,out} = T(p_\mathrm{out}, h_\mathrm{bulk,out}).
\end{equation}

The [!param](/Postprocessors/SCMBulkOutletTemperature/pressure) postprocessor
should provide the same outlet pressure used by the subchannel problem, and
[!param](/Postprocessors/SCMBulkOutletTemperature/fp) should reference the same
fluid-property object.

## Example Input File Syntax

```text
[bulk_temperature_out]
  type = SCMBulkOutletTemperature
  pressure = report_pressure_outlet
  fp = Sodium
  execute_on = "timestep_end"
[]
```

!syntax parameters /Postprocessors/SCMBulkOutletTemperature

!syntax inputs /Postprocessors/SCMBulkOutletTemperature

!syntax children /Postprocessors/SCMBulkOutletTemperature
