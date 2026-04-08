#include <petsc.h>

int main(int argc,char **argv)
{
  PetscErrorCode ierr;
  ierr = PetscInitialize(&argc,&argv,NULL,NULL);if (ierr) return ierr;
  ierr = PetscFinalize();
  return ierr;
}
