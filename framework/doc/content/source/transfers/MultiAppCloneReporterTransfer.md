# MultiAppCloneReporterTransfer

!syntax description /Transfers/MultiAppCloneReporterTransfer

## Overview

This transfer is very similar to [MultiAppReporterTransfer.md] with a few key differences. The first is that this object can only transfer from sub-application(s) to the main app. The second is that reporter values are declared on-the-fly, i.e. the reporter values from the sub-app are "cloned" onto the main app, with out an already existing value on the main app. The third is that when there are multiple sub-applications, the values are collected into a vector of that reporter type on the main app.

!alert warning
It is not possible to transfer a reporter value that has already been cloned with this object. For instance, let's say there is a hierarchical structure of multiapps with a primary (main), secondary, and tertiary app. If this object is used to clone values from the tertiary to secondary, it cannot again be used to clone the same values from the secondary to the primary.

## Example Input Syntax

### Multiple Sub Applications

This example shows a multi-app with multiple sub applications. It uses the same input file, but with different reporter values controlled by [!param](/MultiApps/TransientMultiApp/cli_args):

!listing reporter_transfer/clone.i block=multi_reporter indent=2 header=[MultiApps] footer=[]

The transfer syntax is pretty minimal giving reporter value names (can be found in [sub0.i](reporter_transfer/sub0.i)) on the sub-app and reporter object name on main app, the object name is used to associate the cloned value to an object.

!listing reporter_transfer/clone.i block=multi_rep indent=2 header=[Transfers] footer=[]

When run in serial, a single output file is produced which shows the vectors of the reporter values created:

!listing reporter_transfer/gold/clone_serial.json language=json start=multi_rep:from_sub_pp:value end=multi_vpp:from_sub_vpp:a

In parallel, the sub app reporter values are split into different files, one for each processor. This example was run with 6 processors which split the 4 sub apps. So the root processor for each sub app ([clone_out.json](reporter_transfer/gold/clone_out.json), [clone_out.json.1](reporter_transfer/gold/clone_out.json.1), [clone_out.json.2](reporter_transfer/gold/clone_out.json.2), and [clone_out.json.4](reporter_transfer/gold/clone_out.json.4)) will show the value, while the other processors ([clone_out.json.3](reporter_transfer/gold/clone_out.json.3) and [clone_out.json.5](reporter_transfer/gold/clone_out.json.5)) will be empty:

!listing reporter_transfer/gold/clone_out.json language=json start=multi_rep:from_sub_pp:value end=multi_vpp:from_sub_vpp:a

!listing reporter_transfer/gold/clone_out.json.3 language=json start=multi_rep:from_sub_pp:value end=multi_vpp:from_sub_vpp:a

### Single Sub Application

When there is a only a single sub-application in the multi-app, the reporter value is directly cloned (versus creating a vector):

!listing reporter_transfer/clone.i block=single_app indent=2 header=[MultiApps] footer=[]

!listing reporter_transfer/clone.i block=single indent=2 header=[Transfers] footer=[]

The resulting output in serial and the root processor in parallel will contain the transferred values, while the non-root processors will have the default constructor of the value type:

!listing reporter_transfer/gold/clone_out.json language=json start=single:from_sub_pp:value end=time

!listing reporter_transfer/gold/clone_out.json.1 language=json start=single:from_sub_pp:value end=time

!syntax parameters /Transfers/MultiAppCloneReporterTransfer

!syntax inputs /Transfers/MultiAppCloneReporterTransfer

!syntax children /Transfers/MultiAppCloneReporterTransfer
