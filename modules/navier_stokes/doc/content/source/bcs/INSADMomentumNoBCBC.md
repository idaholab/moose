# INSADMomentumNoBCBC

The `INSADMomentumNoBCBC` class is used to implement the "No Boundary Condition"
developed by
[Griffiths](https://onlinelibrary.wiley.com/doi/abs/10.1002/(SICI)1097-0363(19970228)24:4%3C393::AID-FLD505%3E3.0.CO;2-O). According
to Griffiths, the "No Boundary Condition" is equivalent to imposing the
condition on the original analytical problem that the (p+1)st derivative of a finite element variable of order p
should vanish at a point close to the outflow. This is claimed to reduce error
relative to a natural boundary condition.

!syntax description /BCs/INSMomentumNoBCBCLaplaceForm

!syntax parameters /BCs/INSMomentumNoBCBCLaplaceForm

!syntax inputs /BCs/INSMomentumNoBCBCLaplaceForm

!syntax children /BCs/INSMomentumNoBCBCLaplaceForm
