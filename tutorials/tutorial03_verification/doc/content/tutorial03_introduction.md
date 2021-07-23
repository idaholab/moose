# Verification

!---

## Theory

The term "verification" is rooted in software engineering practices and software quality standards.

The Institute of Electrical and Electronics Engineers (IEEE) defines verification as *"The assurance
that a product, service, or system meets the needs of the customer and other identified
stakeholders. It often involves acceptance and suitability with external customers."*

!---

## Practice

For our purposes, verification is the process of ensuring that that the software is performing calculations
as expected.

For a MOOSE-based application, +ensure that the equations are solved correctly+.

!---

### Process

The process of performing a verification study is simple in practice:

1. Perform simulation with a known solution and measure the error.
2. Refine the simulation (spatially or temporally) and repeat step 1.

If the error is reduced at the expected rate, then your simulation is numerically correct.
