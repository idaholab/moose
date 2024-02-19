!config navigation breadcrumbs=False scrollspy=False

# Getting Started

To use THM, you must either build a MOOSE-based application that includes the
`thermal_hydraulics` module, or you can build the stand-alone THM executable as
follows:

1. [Install MOOSE](getting_started/installation/index.md optional=True).
1. Build THM. Starting from the root MOOSE directory (e.g., `~/projects/moose`),

   ```
   cd modules/thermal_hydraulics
   make -j 10
   ```

   where the number following `-j` is the number of processes you'd like to use.

