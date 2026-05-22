# ShellBlockGSSNESExecutor + NewtonSNESExecutor test.
# Two independent diffusion systems: nl0 and nl1.
# ShellBlockGSSNESExecutor sweeps both systems as the NPC for NewtonSNESExecutor.
# Both systems have the analytic solution u=v=x on [0,1].
# Run with --executor.

!include physics.i
!include nlp.i
