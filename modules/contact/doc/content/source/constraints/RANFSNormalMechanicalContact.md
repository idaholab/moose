# RANFSNormalMechanicalContact

Enforces the zero penetration constraint using the Reduced Active Nonlinear
Function Set (RANFS) scheme. The algorithm is as follows: do a min comparison
between the penetration and the Lagrange Multiplier (LM) associated with the
zero penetration constraint. If the penetration (properly signed) is less than
the LM, then we replace the non-linear residual equation for the secondary node with
the zero-penetration constraint equation (represented simply by the gap). In
this way the gap is required to be zero.

Advantages of the RANFS scheme:

- No Lagrange Multipliers, e.g. no saddle-point and consequently we are free to use preconditioners like AMG
- No penalty term, so no introduction of ill conditioning into the matrix
- Exact satisfaction of the constraints

The `RANFSNormalMechanicalContact` object includes ping-ponging protection. It
detects when a secondary node has alternated back and forth between two different
element faces a sufficient number of times (currently
face1-face2-face1-face2-face1) and then fixes the issue by applying more RANFS
Explicitly, we apply a number of equality constraints equal to the mesh
dimension tying together the locations of the secondary node and the nearest primary
node (which we assert is a node that both primary faces involved in the ping-pong
share). This ping-ponging protection is necessary for solving step 19 of the
[`full-bouncing-block-ranfs`](bouncing-block-contact/bouncing-block-ranfs.i) test. Without the
protection the solve does not converge and the time-step gets cut.

!syntax description /Constraints/RANFSNormalMechanicalContact

!syntax parameters /Constraints/RANFSNormalMechanicalContact

!syntax inputs /Constraints/RANFSNormalMechanicalContact

!syntax children /Constraints/RANFSNormalMechanicalContact
