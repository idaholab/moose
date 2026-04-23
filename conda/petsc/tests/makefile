include ${PETSC_DIR}/lib/petsc/conf/variables
include ${PETSC_DIR}/lib/petsc/conf/rules

ex1: ex1.o
	${CLINKER} -o ex1 ex1.o ${PETSC_LIB}
	${RM} ex1.o

runex1:
	${MPIEXEC} -n 1 ./ex1

include ${PETSC_DIR}/lib/petsc/conf/test
