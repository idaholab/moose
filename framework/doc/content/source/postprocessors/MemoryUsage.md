# MemoryUsage

!syntax description /Postprocessors/MemoryUsage

This postprocessor collects various memory usage metrics:

- physical memory (most relevant, tied to hardware RAM limitation)
- virtual memory
- major page faults (how often disk swap is accessed)

The units for memory default to MegaBytes, but users can report usage in other
units through the mem_units parameter: (bytes, kilobytes, megabytes, gigabytes).

The data can be reduced in parallel as

- maximum out of all MPI ranks
- minimum out of all MPI ranks
- average over all MPI ranks
- total sum over all MPI ranks (default)

Physical memory statistics are available on Mac and Linux, virtual memory and
page fault data is Linux only.

This postprocessor can be executed multiple times per timestep and by default
aggregates the per-process peak value of the chosen metric, which then can be
output on `TIMESTEP_END`.

!alert note
Until October 2018 this Postprocessor defaulted to reporting *virtual memory*.
This was changed to the more relavant *physical memory* to avoid misleading
benchmark results to be generated.

For a VectorPostprocessor that provides detailed per MPI rank memory statistics see
[`VectorMemoryUsage`](/VectorMemoryUsage.md).

## Implementation

The `/proc/self/status` files is checked first. This file only exists on Linux
systems and contains
[several columns](http://man7.org/linux/man-pages/man5/proc.5.html) with process
specific statistics. On mac systems a conditionally compiled code (`#ifdef __APPLE__`)
block uses a mach kernel API function `task_info` to obtain the memory sizes of the
current process.

!syntax parameters /Postprocessors/MemoryUsage

!syntax inputs /Postprocessors/MemoryUsage

!syntax children /Postprocessors/MemoryUsage
