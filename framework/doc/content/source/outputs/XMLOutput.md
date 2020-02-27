# XMLOutput

!syntax description /Outputs/XMLOutput

## Overview

The [VectorPostprocessors/index.md] allows vectors of data to be generated. This data is
traditionally output in the form of CSV files. For large numbers of vectors this can be
problematic for many reasons. Foremost, a CSV file is created for each object and each timestep,
resulting in a large number of output files.

If a single file is desired for all objects and timesteps the XML output can be used. If the
VectorPostprocessor object(s) are being executed in distributed mode there will be a file
created for each processor being utilized.

## Example Input File Syntax

### Replicated Data

If the VectorPostprocessor is defined to execute in "replicated" mode (the default), then
a single XML file will be created. For example, [xml-replicated] enables the XML output that will
result in a single output file (xml_out.xml); the resulting content of this file is shown
below the snippet.

!listing xml.i id=xml-replicated block=VectorPostprocessors Outputs
         caption=Example input file snippet with VectorPostprocessor data with XML output.

!listing outputs/xml/gold/xml_out.xml id=xml-replicated-out
         caption=Output resulting from executing content of [xml-replicated].

Notice the "distributed" object creates a vector "data", but this data is not populated
during initial setup; no data exists with "time=0".

### Distributed Data

In the distributed case, the actual vectors of data within a VectorPostprocessor object only
contain portions of the complete vector on each processor. For example, if [xml-replicated]
is re-executed with three processes and the parallel type is changed to distributed, an XML file
will be created for each processor. Each file will contain the portion of that data
associated with the processor, as shown in the three files below.

!listing outputs/xml/gold/xml_distributed_out.xml

!listing outputs/xml/gold/xml_distributed_out.xml.1

!listing outputs/xml/gold/xml_distributed_out.xml.2

!syntax parameters /Outputs/XMLOutput

!syntax inputs /Outputs/XMLOutput

!syntax children /Outputs/XMLOutput
