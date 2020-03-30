# Rank Two Based Failure Criteria AuxKernel

## Description

The `RankTwoBasedFailureCriteriaNOSPD` AuxKernel is to determine the status of each individual bond using rank two tensors, i.e., strain and stress, based scalar quantities, i.e., axial strain/stress, maximum principal strain/stress, Tresca strain/stress and von Mises strain/stress. It should be noted that the listed failure criteria in this class only applies to non-ordinary state-based model. This auxKernel can be expanded to include other rank two tensor based failure criteria.


!syntax parameters /AuxKernels/RankTwoBasedFailureCriteriaNOSPD

!syntax inputs /AuxKernels/RankTwoBasedFailureCriteriaNOSPD

!syntax children /AuxKernels/RankTwoBasedFailureCriteriaNOSPD
