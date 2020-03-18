include ${PETSC_DIR}/lib/petsc/conf/variables
include ${PETSC_DIR}/lib/petsc/conf/rules

# TODO: Missing Fortran rules fixed in petsc/maint (upcoming petsc-3.12.1)
.F90.o:
	${PETSC_FCOMPILE} -o $@ $<

ex1: ex1.o
	${CLINKER} -o ex1 ex1.o ${PETSC_SNES_LIB}
	${RM} ex1.o

ex1f: ex1f.o
	${FLINKER} -o ex1f ex1f.o ${PETSC_SNES_LIB}
	${RM} ex1f.o

runex1:
	@${MPIEXEC} -n 1 ./ex1 -snes_monitor_short -ksp_gmres_cgs_refinement_type refine_always

runex1f:
	@${MPIEXEC} -n 1 ./ex1f -snes_monitor_short -snes_type newtontr -ksp_gmres_cgs_refinement_type refine_always -nox

include ${PETSC_DIR}/lib/petsc/conf/test
