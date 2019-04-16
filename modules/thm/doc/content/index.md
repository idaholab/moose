!config navigation breadcrumbs=False scrollspy=False

# THM

THM stands for "Thermal-Hydraulic Module" and is a common code base for MOOSE-based
thermal-hydraulic applications utilizing 1-D flows. It provides the following:

- The +Components+ system, which provides a higher-level interface to users for
  creating MOOSE object(s). This is useful for assembling thermal-hydraulic
  network of components such as pipes, heat structures, etc.
- Components supporting 1-phase, variable-area, inviscid compressible flow.
- Components supporting 2-phase, variable-area, inviscid compressible flow via
  the "7-equation" model.
- Various classes supporting THM-based applications.
