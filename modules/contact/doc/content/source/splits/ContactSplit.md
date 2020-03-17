# ContactSplit

## Description

The `ContactSplit` class is used to create a split-based preconditioner for contact problems. The idea is to separate the DOFs on the contact interface from the DOFs in the rest of the domain. This allows users to apply different preconditioners for each DOF type. As a common practice, a direct preconditioner (e.g., LU) is often utilized for the interface problem, where the problem size is relatively small while the condition number is large. An iterative preconditioner (e.g., boomeramg) is often employed for the interior problem, where the problem is often better conditioned while the problem size is usually large.

!syntax description /Preconditioning/ContactSplit

## Example Input Syntax

!listing test/tests/fieldsplit/2blocks3d.i block=Preconditioning

!syntax parameters /Preconditioning/ContactSplit

!syntax inputs /Preconditioning/ContactSplit

!syntax children /Preconditioning/ContactSplit
