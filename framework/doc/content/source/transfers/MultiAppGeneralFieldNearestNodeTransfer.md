# MultiAppGeneralFieldNearestNodeTransfer

!alert construction title=Undocumented Class
The MultiAppGeneralFieldNearestNodeTransfer has not been documented. The content listed below should be used as a starting point for
documenting the class, which includes the typical automatic documentation associated with a
MooseObject; however, what is contained is ultimately determined by what is necessary to make the
documentation clear for users.

!syntax description /Transfers/MultiAppGeneralFieldNearestNodeTransfer

## Overview

!alert warning
If [!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/num_nearest_points) is more than 1, the results
will differ in parallel if the target locations are near the parallel process boundaries
on the origin app mesh. Use the [!param](/Debug/SetupDebugAction/output_process_domains) parameter to examine
process boundaries on Exodus/Nemesis output.

## Example Input File Syntax

!! Describe and include an example of how to use the MultiAppGeneralFieldNearestNodeTransfer object.

!syntax parameters /Transfers/MultiAppGeneralFieldNearestNodeTransfer

!syntax inputs /Transfers/MultiAppGeneralFieldNearestNodeTransfer

!syntax children /Transfers/MultiAppGeneralFieldNearestNodeTransfer
