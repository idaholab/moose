# Nemesis

!syntax description /Outputs/Nemesis

## Overview

The Nemesis output is the parallel version of the [ExodusII](Exodus.md) format, which should generally
be used when running a simulation with a distributed mesh. See [Mesh/index.md] for
more information.

Nemesis files are output by MOOSE with a `.n` extension. They can be read/detected as Nemesis files with a
`.n` or `.nem` extension.


!syntax parameters /Outputs/Nemesis

!syntax inputs /Outputs/Nemesis

!syntax children /Outputs/Nemesis
