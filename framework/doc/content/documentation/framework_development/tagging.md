## Tagging System
In finite element calculations, for each element we need to compute physics mapping (from a reference element to a physics element), basis functions, derivatives of the basis,  materials and other element related information. This computation may be expensive, and it becomes even worse when multiple global vectors/matrices need to be filled because every single element has to be visited  multiple times. The basic idea of the tagging system is to resole this challenging issue. Every element is visited once only, and during the visit, all kernels (corresponding to PDE operators) are
evaluated. The local element residual/matrix for the current kernel will be added/inserted/cached to multiple targeted global vectors/matrices. Each kernel can contribute to multiple global vectors/matrices, and which vector/matrix the current kernel should contribute to is implemented by assigning tags. There are a few tags in a kernel, and the kernel calculation will accumulate local contribution to the global counterparts. Obviously, one global vector/matrix is contributed by multiple kernels as well.

## Design
All kernel-like objects inherit from [TaggingInterface](/TaggingInterface.md),  and they can be assigned to different tags through parameters *_vector_tags_* and *_matrix_tags_*. By default, "nontime" is set. The multiple local residuals/matrices are assembled by *_Assembly Object_* to their global counterparts that then are used in *_NonlinearSystem_* and *_FEPRoblem_*. Data flow is shown as follows:

!media media/framework/tagging/tagging_flow_chart.png style="width:50%;padding-left:20px;float:right" caption=Tagging Flow Chart



In Kernel, we store multiple copies of the local residual/matrix, and these copies will be added to global vectors and matrices.
