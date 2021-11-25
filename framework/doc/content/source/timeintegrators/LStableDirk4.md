# LStableDirk4

!syntax description /Executioner/TimeIntegrator/LStableDirk4

The Butcher tableau for this method is:

!table
1/4 |  1/4
0   | -1/4 1/4
1/2 |  1/8 1/8 1/4
1   | -3/2 3/4 3/2   1/4
1   |    0 1/6 2/3 -1/12 1/4
----------------------------
1   |    0 1/6 2/3 -1/12 1/4

The stability function for this method is:

!equation
R(z) = -(28*z^4 + 32*z^3 - 384*z^2 - 768*z + 3072)/
        (3*z^5 - 60*z^4 + 480*z^3 - 1920*z^2 + 3840*z - 3072)

The method is L-stable:

!equation
lim_{z->\infnty} R(z) = 0

Notes:
The method was found in [!cite](skvortsov2006) but it may not be the original source.
There is also a 4th-order rule with 5 stages on page 107 of [!cite](hairer1999)
but its coefficients have less favorable "amplification factors" than the present rule.

!syntax parameters /Executioner/TimeIntegrator/LStableDirk4

!syntax inputs /Executioner/TimeIntegrator/LStableDirk4

!syntax children /Executioner/TimeIntegrator/LStableDirk4
