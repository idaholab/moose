# ComponentsConvergence

This [IterationCountConvergence.md] can be used with the [Components system](Components/index.md) to check the convergence of all Components that return a Convergence object with `getNonlinearConvergence()`. The inner convergence check returns its convergence status as follows:

- `CONVERGED`: All Components returning a Convergence object are `CONVERGED`
- `DIVERGED`: Any Component returning a Convergence object is `DIVERGED`
- `ITERATING`: Not `CONVERGED` nor `DIVERGED`

!syntax parameters /Convergence/ComponentsConvergence

!syntax inputs /Convergence/ComponentsConvergence

!syntax children /Convergence/ComponentsConvergence
