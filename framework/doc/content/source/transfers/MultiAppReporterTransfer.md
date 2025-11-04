# MultiAppReporterTransfer

!syntax description /Transfers/MultiAppReporterTransfer

## Overview

This MultiAppReporterTransfer provides a method to transfer a reporter value (see [reporters](/Reporters/index.md)) of any type between the main application and the sub-application(s).  This includes vectors of real numbers from [vectorpostprocessors](/VectorPostprocessors/index.md) as well as real numbers from [postprocessors](/Postprocessors/index.md).

[!param](/Transfers/MultiAppReporterTransfer/from_reporters) specifies where the data is coming from, and [!param](/Transfers/MultiAppReporterTransfer/to_reporters) specifies where the data going to. These are a list of reporter names that must be the same length, as they directly correspond to each other. For [vectorpostprocessors](/VectorPostprocessors/index.md) the syntax is "vpp_name"/"vector_name" and for [reporters](/Reporters/index.md) the syntax is "reporter_name"/"value_name".

When transferring data from the main application the data from the main is copied to each sub-application. If the [!param](/Transfers/MultiAppReporterTransfer/subapp_index) is used then data is only transferred to the specified sub-application. When transferring data to the main application the [!param](/Transfers/MultiAppReporterTransfer/subapp_index) must be supplied if there is more than one sub-application.

## Siblings transfer behavior

This transfer supports sending data from a MultiApp to a MultiApp if and only if the number of subapps
in the source MultiApp matches the number of subapps in the target MultiApp, and they are distributed
the same way on the parallel processes. Each source app is then matched to the target app with the same
subapp index.

## Distributed Vector Transfer

The MultiAppReporterTransfer also supports transferring reporter vectors in a distributed fashion using the [!param](/Transfers/MultiAppReporterTransfer/distribute_reporter_vector) parameter.
In this mode, the transfer assumes a one-to-many or many-to-one relationship
between the reporter values in the main application and the sub-applications.
The reporter value in the main application is expected to be a vector with a
size matching the number of sub-applications. Each sub-application will
send/receive its respective component of the vector based on its index. The main
app reporter is assumed to be replicated while the subapp reporters are assumed
to be root or replicated.

!table caption=Distributed Transfer Examples
| Main App | Sub App 1 | Sub App 2 | Sub App 3 | Sub App 4 |
|---|---|---|---|---|
| $\begin{pmatrix}1\\2\\3\\4\end{pmatrix}$ | 1 | 2 | 3 | 4 |
|
| $\begin{pmatrix}1\\2\end{pmatrix}$ $\begin{pmatrix}3\\4\\5\end{pmatrix}$ $\begin{pmatrix}6\\7\\8\\9\end{pmatrix}$ $\begin{pmatrix}10\\11\\12\end{pmatrix}$ | $\begin{pmatrix}1\\2\end{pmatrix}$ | $\begin{pmatrix}3\\4\\5\end{pmatrix}$ | $\begin{pmatrix}6\\7\\8\\9\end{pmatrix}$ | $\begin{pmatrix}10\\11\\12\end{pmatrix}$ |

!alert note title=[!param](/Transfers/MultiAppReporterTransfer/distribute_reporter_vector)  is only implemented for certain reporter modes.
The main
app reporter is assumed to be `REPORTER_MODE_REPLICATED` while the subapp reporters are assumed
to be `REPORTER_MODE_ROOT` or `REPORTER_MODE_REPLICATED`. This operation will
error out with any reporter that
are `REPORTER_MODE_DISTRIBUTED`.

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



### Distribution of a vector Reporter

#### Scatter vector Reporter

Here we are transferring a vector reporter and a vector of vectors reporter in a
scatter fashion. The main application holds a vector of vectors and a
single vector, while each subapp has a vector and a Real (scalar) reporter.
During the transfer the subapp's reporters are populated from the scattering of
the main app's vector reporter. The main app can also use a vector of vectors reporter, in which case those vectors
are distributed to the subapps.

!listing dist_vector/main_send.i block=Reporters caption=Main application reporters

!listing dist_vector/sub.i block=Reporters caption=Sub-application reporters

!listing dist_vector/main_send.i block=Transfers caption=Main application reporter transfers
   indent=2 header=[Transfers] footer=[]


#### Gather vector Reporter

Here we are transferring a vector reporter and a vector of vectors reporter in a gather fashion. The main application holds a vector of vectors and a single vector, while each subapp has a vector and a Real (scalar) reporter. This test shows the gather operation by aggregating the same values from each subapp into a vector of reporters in the main application.

!listing dist_vector/main_rec.i block=Reporters caption=Main application reporters

!listing dist_vector/sub.i block=Reporters caption=Sub-application reporters

!listing dist_vector/main_rec.i block=Transfers caption=Main application reporter transfers
   indent=2 header=[Transfers] footer=[]

!syntax parameters /Transfers/MultiAppReporterTransfer

!syntax inputs /Transfers/MultiAppReporterTransfer

!syntax children /Transfers/MultiAppReporterTransfer
