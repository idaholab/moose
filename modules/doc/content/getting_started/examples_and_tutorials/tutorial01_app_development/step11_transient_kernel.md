!!!
ANTICIPATED OUTLINE OF NEXT FEW STEPS

11: Transient Kernel (steady heat conduction and its time derivative)
12: BCs (heat conduction outflow)
13: Functions (necessary prerequisite for step 14)
14: Equation Coupling (darcy advection)

- definitely need to get some physics modules action in after these. It'd be super cool if we could couple in tensormechanics and compute thermal strains and even mechanical contact. E.g., maybe we model some sort of expansion joint connecting one end of the pipe with the adjacent pressure vessel. Contact module seems to be very robust these days.
!!!

# Step 11: Develop a Time-dependent Kernel Object

!alert construction
The remainder of this tutorial is currently being developed. More content should be available soon. For now, refer back to the [examples_and_tutorials/index.md] page for other helpful training materials or check out the MOOSE [application_development/index.md] pages for more information.

!content pagination previous=tutorial01_app_development/step10_auxkernels.md

!!!
The workshop breaks this step into two parts, steady-state and transient heat conduction, but I think these can be appropriately consolidated into one step since we've already introduced the basic concepts of a steady-state kernel.

The workshop also does a whole spiel on Executioners. While these concepts are important, I think they only need to be briefly mentioned in the body text here. The only major difference in the Executioner is changing the type to Transient. For such a simple problem, no one needs to worry about time-integration/differentiation and adaptive steppers. This is just the kind of stuff that need not be pulled out from under the hood in an introductory MOOSE tutorial IMO.

P.S., probably ought to mention something about the exec flags and the common `execute_on` param here.
!!!
