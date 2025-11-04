# TimedSubdomainModifier

!syntax description /MeshModifiers/TimedSubdomainModifier

## Overview

For geometrically complex models, an efficient option is to define the volumes to be excavated/backfilled by means of subdomains (aka 'blocks'). This leads to the task of moving all elements from one subdomain to another at pre-known points in time. Furthermore, in some engineering applications
(e.g. geotechnics or mining) it might be necessary to reassign several subdomains during one simulation.
Addressing this task, the `TimedSubdomainModifier` eases re-assignment of all elements of a subdomain, makes Moose input files shorter, and reduces
potential sources of error compared to the usage of [CoupledVarThresholdElementSubdomainModifier.md] (which in turn has its advantages if the geometry is not organized into subdomains).

The `TimedSubdomainModifier` inherits from the [ElementSubdomainModifier.md]. Details on solution reinitialization, stateful material property reinitialization and moving boundary/interface nodeset/sideset modification can be found in the description of the [ElementSubdomainModifier.md].

The subdomains and times to be used by the `TimedSubdomainModifier` can be specified using one of the following options:

- 3 vectors containing the times, source and destination subdomains/blocks, or
- path to an CSV file

## Example Input File Syntax

### Vectors of data in the input file

If the data on the subdomains and times should be provided directly in the input file, the following 3 parameters must be used. The vector of data in each parameter must have the same number of items:

- [!param](/MeshModifiers/TimedSubdomainModifier/times): Vector of times at which the elements are to be moved.
- [!param](/MeshModifiers/TimedSubdomainModifier/blocks_from): Vector of subdomain/blocks to move from. The subdomains/blocks may be given as ID or name.
- [!param](/MeshModifiers/TimedSubdomainModifier/blocks_to): Vector of subdomain/blocks to move to. The subdomains/blocks may be given as ID or name.

!listing test/tests/meshmodifiers/element_subdomain_modifier/tsm_direct.i block=MeshModifiers

### Reading data from CSV file

To read the data on the subdomains and times from an CSV file, the following parameters are to be used:

- [!param](/MeshModifiers/TimedSubdomainModifier/data_file): Name of the file in which the data is read.
- [!param](/MeshModifiers/TimedSubdomainModifier/delimiter): Optional CSV delimiter character. Defaults to comma (`,`).
- [!param](/MeshModifiers/TimedSubdomainModifier/comment): Optional CSV comment character. Defaults to hash character (`;`).
- [!param](/MeshModifiers/TimedSubdomainModifier/header): This parameter must be set to True, if the columns are to be found via header ([!param](/MeshModifiers/TimedSubdomainModifier/time_column_text), [!param](/MeshModifiers/TimedSubdomainModifier/blocks_from_column_text), and [!param](/MeshModifiers/TimedSubdomainModifier/blocks_to_column_text)). See following parameters.
- [!param](/MeshModifiers/TimedSubdomainModifier/time_column_index) -or- [!param](/MeshModifiers/TimedSubdomainModifier/time_column_text): Zero-based index or name of the column defining the times.
- [!param](/MeshModifiers/TimedSubdomainModifier/blocks_from_column_index) -or- [!param](/MeshModifiers/TimedSubdomainModifier/blocks_from_column_text): Zero-based index or name of the column defining the subdomains/blocks to move all elements from.
- [!param](/MeshModifiers/TimedSubdomainModifier/blocks_to_column_index) -or- [!param](/MeshModifiers/TimedSubdomainModifier/blocks_to_column_text): Zero-based index or name of the column defining the subdomains/blocks to move all elements to.

!listing test/tests/meshmodifiers/element_subdomain_modifier/tsm_csv.i block=MeshModifiers

With the corresponding CSV-file:

!listing test/tests/meshmodifiers/element_subdomain_modifier/tsm.csv

!syntax parameters /MeshModifiers/TimedSubdomainModifier

!syntax inputs /MeshModifiers/TimedSubdomainModifier

!syntax children /MeshModifiers/TimedSubdomainModifier
