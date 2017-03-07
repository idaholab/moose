# MemoryUsage
!description /Postprocessors/MemoryUsage

This postprocessor collects various memory usage metrics:

* physical memory (most relevant)
* virtual memory
* page faults (how often disk swap is accessed)

The data can be reduced in parallel as

* maximum out of all MPI ranks
* minimum out of all MPI ranks
* average over all MPI ranks
* total sum over all MPI ranks

Physical memory statistics are available on Mac and Linux, virtual memory and
page fault data is Linux only.

This postporcessor can be executed multiple times per timestep and by default
aggregates the per-process peak value of the chosen metric, which then cam be output
on `TIMESTEP_END`.

!parameters /Postprocessors/MemoryUsage

!inputfiles /Postprocessors/MemoryUsage

!childobjects /Postprocessors/MemoryUsage
