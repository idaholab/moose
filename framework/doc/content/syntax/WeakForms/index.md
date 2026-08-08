# WeakForms

The `WeakForms` syntax is used to create MFEM `MFEMWeakFormBase`-derived objects to set up MFEM
[EquationSystem.md] operators that can be used by downstream solvers and preconditioners. If no
`WeakForms` block has been added to an input file by the user, a default  `MFEMWeakFormBase`-derived
object will be added to the system, based on the `NumericType` (real or complex) used by the
`MFEMProblem`, and whether the Executioner in use is transient or steady state.
