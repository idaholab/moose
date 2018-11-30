# Backup

The Backup object is a simply a `struct` for holding binary blob "checkpoint" data. The object contains a stream for holding
global simulation state data along with a separate vector for holding individual thread state for each thread in the simulation.
The Backup object is part of the larger [Restart/Recovery](restart_recover.md optional=True) system in MOOSE.

The Backup object contains the serialized data from MOOSE's `dataLoad/dataStore` routines found in [DataIO.h](/DataIO.h).
