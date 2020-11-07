# MultiAppReporterTransfer

!syntax description /Transfers/MultiAppVectorReporterTransfer

## MultiAppVectorReporterTransfer id=vector_transfer

This MultiAppVectorReporterTransfer provides a method to transfer complete vectors of data between the main application and the sub-applications. This includes vectors of real numbers from [vectorpostprocessors](/VectorPostprocessors/index.md) or [reporters](/Reporters/index.md).

[!param](/Transfers/MultiAppVectorReporterTransfer/from_reporters) specifies where the data is coming from, and [!param](/Transfers/MultiAppVectorReporterTransfer/to_reporters) specifies where the data going to. These are a list of reporter names that must be the same length, as they directly correspond to each other. For [vectorpostprocessors](/VectorPostprocessors/index.md) the syntax is "vpp_name"/"vector_name" and for [reporters](/Reporters/index.md) the syntax is "reporter_name"/"value_name".

When transferring data from main application the vector from the main is copied to each of the sub-applications. If the [!param](/Transfers/MultiAppVectorReporterTransfer/subapp_index) is used then data is only transferred to the specified sub-application. When transferring data to the main application the [!param](/Transfers/MultiAppVectorReporterTransfer/subapp_index) must be supplied if there is more than one sub-application.

## MultiAppRealReporterTransfer id=real_transfer

This MultiAppRealReporterTransfer provides a method to transfer real values between the main application and sub-application(s). This includes real numbers from [postprocessors](/Postprocessors/index.md) or [reporters](/Reporters/index.md).

[!param](/Transfers/MultiAppVectorReporterTransfer/from_reporters) specifies where the data is coming from, and [!param](/Transfers/MultiAppRealReporterTransfer/to_reporters) specifies where the data going to. These are a list of reporter names that must be the same length, as they directly correspond to each other. For [postprocessors](/Postprocessors/index.md) the syntax is "pp_name"/`value` and for [reporters](/Reporters/index.md) the syntax is "reporter_name"/"value_name".

When transferring data from main application the vector from the main is copied to each of the sub-applications. If the [!param](/Transfers/MultiAppRealReporterTransfer/subapp_index) is used then data is only transferred to the specified sub-application. When transferring data to the main application the [!param](/Transfers/MultiAppRealReporterTransfer/subapp_index) must be supplied if there is more than one sub-application.

## MultiAppIntegerReporterTransfer id=int_transfer

This MultiAppIntegerReporterTransfer provides a method to transfer integer values from [reporters](/Reporters/index.md) between the main application and sub-application(s). When transferring data from main application the vector from the main is copied to each of the sub-applications. If the [!param](/Transfers/MultiAppIntegerReporterTransfer/subapp_index) is used then data is only transferred to the specified sub-application. When transferring data to the main application the [!param](/Transfers/MultiAppIntegerReporterTransfer/subapp_index) must be supplied if there is more than one sub-application.

## MultiAppStringReporterTransfer id=string_transfer

This MultiAppStringReporterTransfer provides a method to transfer string values from [reporters](/Reporters/index.md) between the main application and sub-application(s). When transferring data from main application the vector from the main is copied to each of the sub-applications. If the [!param](/Transfers/MultiAppStringReporterTransfer/subapp_index) is used then data is only transferred to the specified sub-application. When transferring data to the main application the [!param](/Transfers/MultiAppStringReporterTransfer/subapp_index) must be supplied if there is more than one sub-application.

## Example Input File Syntax

!alert! tip

You can initialize arbitrary data containers for [!param](/Transfers/MultiAppVectorReporterTransfer/to_reporters) with the following objects:

- Postprocessor: [Receiver.md]
- VectorPostprocessor: [ConstantVectorPostprocessor.md]
- Reporter: [ConstantReporter.md]

!alert-end!

### Tranferring VectorPostprocessors

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

### Tranferring Reporter Real Numbers

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

!syntax parameters /Transfers/MultiAppVectorReporterTransfer

!syntax inputs /Transfers/MultiAppVectorReporterTransfer

!syntax children /Transfers/MultiAppVectorReporterTransfer
