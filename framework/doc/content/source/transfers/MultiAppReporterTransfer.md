# MultiAppReporterTransfer

!syntax description /Transfers/MultiAppReporterTransfer

## Overview

This MultiAppReporterTransfer provides a method to transfer a reporter value (see [reporters](/Reporters/index.md)) of any type between the main application and the sub-application(s).  This includes vectors of real numbers from [vectorpostprocessors](/VectorPostprocessors/index.md) as well as real numbers from [postprocessors](/Postprocessors/index.md).

[!param](/Transfers/MultiAppReporterTransfer/from_reporters) specifies where the data is coming from, and [!param](/Transfers/MultiAppReporterTransfer/to_reporters) specifies where the data going to. These are a list of reporter names that must be the same length, as they directly correspond to each other. For [vectorpostprocessors](/VectorPostprocessors/index.md) the syntax is "vpp_name"/"vector_name" and for [reporters](/Reporters/index.md) the syntax is "reporter_name"/"value_name".

When transferring data from the main application the data from the main is copied to each sub-application. If the [!param](/Transfers/MultiAppReporterTransfer/subapp_index) is used then data is only transferred to the specified sub-application. When transferring data to the main application the [!param](/Transfers/MultiAppReporterTransfer/subapp_index) must be supplied if there is more than one sub-application.

## Example Input File Syntax

!alert! tip

You can initialize arbitrary data containers for [!param](/Transfers/MultiAppReporterTransfer/to_reporters) with the following objects:

- Postprocessor: [Receiver.md]
- VectorPostprocessor: [ConstantVectorPostprocessor.md]
- Reporter: [ConstantReporter.md]

!alert-end!

### Transferring VectorPostprocessors

Here, we are transferring data between vectorpostprocessors:

!listing reporter_transfer/main.i block=VectorPostprocessors caption=Main application VPPs

!listing reporter_transfer/sub0.i block=VectorPostprocessors caption=Sub-application VPPs

!listing reporter_transfer/main.i block=vpp_to_vpp vpp_from_vpp caption=Main application VPP transfers
   indent=2 header=[Transfers] footer=[]

### Transferring Reporter Vectors

Here, we are transferring data between a reporter vector and vectorpostprocessors:

!listing reporter_transfer/main.i block=Reporters caption=Main application reporters

!listing reporter_transfer/sub0.i block=VectorPostprocessors caption=Sub-application VPPs

!listing reporter_transfer/main.i block=vector_to_vpp vector_from_vpp caption=Main application vector transfers
   indent=2 header=[Transfers] footer=[]

### Transferring Reporter Real Numbers

Here, we are transferring data between a reporter real number and postprocessors:

!listing reporter_transfer/main.i block=Reporters caption=Main application reporters

!listing reporter_transfer/sub0.i block=Postprocessors caption=Sub-application Postprocessors

!listing reporter_transfer/main.i block=real_from_pp real_to_pp caption=Main application real transfers
   indent=2 header=[Transfers] footer=[]

### Transferring Reporter Integers and Strings

Here, we are transferring integer and string data between reporters:

!listing reporter_transfer/main.i block=Reporters caption=Main application reporters

!listing reporter_transfer/sub0.i block=Reporters caption=Sub-application reporters

!listing reporter_transfer/main.i block=int_to_int int_from_int string_from_string string_to_string caption=Main application reporter transfers
   indent=2 header=[Transfers] footer=[]

!syntax parameters /Transfers/MultiAppReporterTransfer

!syntax inputs /Transfers/MultiAppReporterTransfer

!syntax children /Transfers/MultiAppReporterTransfer
