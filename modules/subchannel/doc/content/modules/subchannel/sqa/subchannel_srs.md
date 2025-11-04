!template load file=sqa/module_srs.md.template category=subchannel module=SubChannel

!template! item key=system-scope
!! system-scope-begin
The scope of SCM is the single-phase modeling, of ducted subchannel sub-assemblies, with bare pins in a
square arrangement enclosed in a square duct, or wire-wrapped/bare pins, in a triangular arrangement enclosed in a hexagonal duct. In the first case, the coolant is water, in the second case it can be various liquid metals
(Lead,Lead-Bismuth Eutectic, Sodium, Sodium–Potassium) or water.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the Subchannel Module (SCM) is to equip the MOOSE family of codes with a modern, fast and efficient subchannel solver. As a MOOSE module, SCM seamlessly couples with other MOOSE modules and applications, providing intermediate fidelity thermal-hydraulic solutions in a multiphysics context.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
!! assumptions-and-dependencies-begin
SCM is not a finite element code. It connects to the MOOSE framework through the external problem functionality, utilizing the framework's mesh-building capabilities to project its solution onto a compatible MOOSE mesh. The solution is computed using the PETSc library of objects and solvers. Any physics-based or mathematics-based assumptions in code simulations and code objects are detailed in their respective documentation pages.
!! assumptions-and-dependencies-finish
!template-end!

!template! item key=reliability
The regression test suite will cover at least 87% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!
