# SerializedSolutionTransfer

!syntax description /Transfers/SerializedSolutionTransfer

## Overview

This object is responsible to transfer solution fields stored in a [SolutionContainer.md] in
a sub-application to a [ParallelSolutionStorage.md]. The user can specify these objects using
[!param](/Transfers/SerializedSolutionTransfer/parallel_storage) and [!param](/Transfers/SerializedSolutionTransfer/solution_container)
parameters. The solutions in [SolutionContainer.md] are distributed vectors
using the communicator of the sub-application. Based on the value of the [!param](/Transfers/SerializedSolutionTransfer/serialize_on_root) parameter, this object transfers these solution fields in the following ways:

- +If [!param](/Transfers/SerializedSolutionTransfer/serialize_on_root) is disabled+: It distributes the samples between every rank of the sub-application
  and serializes the solution vector on the rank which is supposed to own the sample. This approach results in more balanced
  workloads but requires additional effort due to misalignment with the design of Sampler-based reporters. This might result in significantly increased communication costs.

- +If [!param](/Transfers/SerializedSolutionTransfer/serialize_on_root) is enabled+:
  The solution vectors are serialized and stored on the rank which
  corresponds to root processor of the subapp. This aligns more with the design of the Sampler-based
  reporters, but can cause significant imbalances in the workload if multiple processors are used for the
  sub-applications.

## Example Syntax

The following input file snippet shows how to use the serialized solution transfer to
extract variables `u` and `v` from the data stored on the apps within MultiApp `worker` into a
parallel storage reporter with a name of `parallel_storage`.

!listing test/tests/transfers/serialized_solution_transfer/sst_main.i block=Transfers

## Syntax

!syntax parameters /Transfers/SerializedSolutionTransfer

!syntax inputs /Transfers/SerializedSolutionTransfer

!syntax children /Transfers/SerializedSolutionTransfer
