# Tagging System

In finite element calculations, for each element we need to compute physics mapping (from a reference element to a physics element), basis functions, derivatives of the basis,  materials and other element related information. This computation may be expensive, and it becomes even worse when multiple global vectors/matrices need to be filled because every single element has to be visited multiple times. The basic idea of the tagging system is to resolve this challenging issue.

With the tagging system every element is visited only once.  During this visit all Kernels (corresponding to PDE operators) are evaluated. The local element residual/matrix for the current Kernel will be added/inserted/cached to multiple targeted global vectors/matrices. Each Kernel can contribute to multiple global vectors/matrices, and which vector/matrix the current Kernel should contribute to is implemented by assigning tags. When multiple tags are assigned to a Kernel, the calculation will accumulate the local contribution to multiple matrices/vectors simultaneously.

## Design

All Kernel-like objects (anything computing residuals or Jacobians) inherits from [TaggingInterface](/TaggingInterface.md),  and they can be assigned to different tags through parameters `vector_tags` and `matrix_tags`. The multiple local residuals/matrices are assembled by the Assembly object to their global counterparts that then are used in NonlinearSystem and FEProblem. Data flow is shown as follows:

!media media/framework/tagging/tagging_flow_chart.png style="width:50%;padding-left:20px;float:right" caption=Tagging Flow Chart

In Kernel, we store multiple copies of the local residual/matrix, and these copies will be added to global vectors and matrices.
