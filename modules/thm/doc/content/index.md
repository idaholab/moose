!config navigation breadcrumbs=False scrollspy=False

# THM

THM stands for "Thermal Hydraulics Module" and is a common code base for 1-D thermal
hydraulic applications based on the [MOOSE framework](https://mooseframework.inl.gov/).

- The [Components](syntax/Components/index.md) system, which provides a
  higher-level interface to users for creating MOOSE object(s). This is useful
  for assembling thermal-hydraulic network of components such as pipes, heat
  structures, etc.
- Components supporting 1-phase, variable-area, inviscid, compressible flow.
- Components supporting 2-phase, variable-area, inviscid, compressible flow via
  the "7-equation" model.
- Various classes supporting THM-based applications.
