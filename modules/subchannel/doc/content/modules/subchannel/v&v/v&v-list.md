# Verification and Validation

For software quality assurance purposes, SCM undergoes continuous verification and validation. Verification is the process through which we verify, that SCM solves the equations it sets out to solve, properly. In practical terms,
that is often achieved by comparing the SCM solution, to a known analytical solution for a simple problem case. Validation is the process through which we validate, that the SCM solutions are an accurate representation of reality.
In practical terms, this is achieved by comparing the SCM solution to experimental data.

Note that in addition to monitoring SCM performance and reproducibility in verification and validation cases, the effects of changes made to SCM are tracked. A series of automated tests are performed via continuous integration using CIVET to help identify any changes in SCM's performance,
therefore ensuring stability and robustness.

The test folder in SCM contains the suite of all problems that are used for regression and code coverage testing.
The example folder in SCM contains various examples cases for how to use the code. This includes verification and validation cases as well as other examples.
Finally, a list of publications supporting SCM development can be found [here](list_of_publications.md).

## List of verification cases

### Verification of physics models

| Case | Title |
| :- | :- |
| 1 | [Friction Model, Verification](friction.md) |
| 2 | [Enthalpy Mixing Model, Verification](enthalpy.md) |

## List of validation cases

### Water benchmarks

| Case | Title |
| :- | :- |
| 1 | [PSBT $5\times5$-pin, Validation](PSBT.md) |
| 2 | [PNNL $2\times6$-pin, Validation](pnnl_12_pin.md) |
| 3 | [PNNL $7\times7$-pin sleeve blockage, Validation](pnnl_blockage.md) |

### Liquid Sodium benchmarks

| Case | Title |
| :- | :- |
| 1 | [ORNL $19$-pin, Validation](ornl_19_pin.md) |
| 2 | [Toshiba $37$-pin, Validation](toshiba_37_pin.md) |
| 3 | [EBR-II, SHRT-17,SHRT-45R, Validation](EBR-II.md) |
| 4 | [ORNL Thermal-Hydraulic Out-of-Reactor Safety (THORS) Facility blockage, Validation](thors.md) |
