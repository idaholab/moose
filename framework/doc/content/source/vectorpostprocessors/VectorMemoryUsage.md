# VectorMemoryUsage

!syntax description /VectorPostprocessors/VectorMemoryUsage

This vector postprocessor generates multiple output columns with one row per MPI rank

| Column name   | Description |
|---------------|--------------|
| `hardware_id` | Ranks with a common hardware ID share common RAM (i.e. are located on the same compute node)|
| `total_ram`   | Total available RAM on the compute node the respective MPI rank is located on.|
| `physical_mem` | Physical memory the current rank uses (default unit: MBs).|
| `virtual_mem` | Virtual memory the current rank uses (the amount returned strongly depends on the operating system and does not reflect the physical RAM used by the simulation - default unit: MBs).|
| `page_faults` | Number of hard page faults encountered by the MPI rank. This number is only available on Linux systems and indicates the amount of swap activity (indicating low performance due to insufficient available RAM)|
| `node_utilization` | Indicates which fraction of the total RAM available on the compute node is occupied by MOOSE processes of the current simulation.|

For a Postprocessor that provides min/max/average memory statistics see
[`MemoryUsage`](/MemoryUsage.md).

## Example visualizations

This VectorPostprocessor can be visualized using the
[`VectorPostprocessorVisualizationAux`](/VectorPostprocessorVisualizationAux.md)
AuxKernel.
Both the IDs:

!media media/vectorpostprocessors/ss_grid.png style=width:32%;margin-left:30px;float:left;
    caption=Refined grid

!media media/vectorpostprocessors/ss_processor_id.png style=width:32%;margin-left:30px;float:left
    caption=Processor id (using [`ProcessorIDAux`](/ProcessorIDAux.md))

!media media/vectorpostprocessors/ss_hardware_id.png style=width:32%;margin-left:30px;float:left
    caption=Hardware id (i.e. compute node)


and the memory used:


!media media/vectorpostprocessors/ss_physical_mem.png style=width:32%;margin-left:30px;float:left
    caption=Physical memory used by rank

!media media/vectorpostprocessors/ss_node_utilization.png style=width:32%;margin-left:30px;float:left
    caption=Fraction of RAM used by the current simulation on the compute node


!syntax parameters /VectorPostprocessors/VectorMemoryUsage

!syntax inputs /VectorPostprocessors/VectorMemoryUsage

!syntax children /VectorPostprocessors/VectorMemoryUsage
