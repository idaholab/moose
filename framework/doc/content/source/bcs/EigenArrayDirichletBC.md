# EigenArrayDirichletBC

It is an array version of EigenDirichletBC provided for Eigenvalue solvers to
handle Dirichlet boundary conditions for the right-hand side. It is different
from regular DirichletBC. EigenArrayDirichletBC will always return 0
regardless of the residual. The corresponding rows of the matrix are zeroed
out without adding ones to the diagonal.

!syntax description /BCs/EigenDirichletBC

!syntax parameters /BCs/DirichletBC

!syntax inputs /BCs/DirichletBC

!syntax children /BCs/DirichletBC
