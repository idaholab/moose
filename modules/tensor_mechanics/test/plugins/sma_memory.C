/***************************************************************************************
**  UMAT, FOR ABAQUS/STANDARD INCORPORATING ISOTROPIC ELASTICITY                      **
***************************************************************************************/

#include "SMAAspUserSubroutines.h"
#include "MooseError.h"

extern "C" void
uexternaldb_(
    int * LOP, int * /* LRESTART */, Real /* TIME */[], Real * /* DTIME */, int * KSTEP, int * KINC)
{
  switch (*LOP)
  {
    // beginning of the simulation
    case 0:
    {
      Real * fa = SMAFloatArrayCreate(1, 5, 1.1);
      Real fasum = 0.0;
      for (unsigned int i = 0; i < 5; ++i)
        fasum += fa[i];

      // test wrong handle error based on kstep setting
      Real * fa2 = SMAFloatArrayAccess(*KSTEP == 1 ? 2 : 1);
      if (fa != fa2)
        mooseError("Mismatching pointers");

      int * ia = SMAIntArrayCreate(67, 39, 9);
      int iasum = 0;
      for (unsigned int i = 0; i < 39; ++i)
        iasum += ia[i];

      // test wrong handle error based on kstep setting
      int * i2 = SMAIntArrayAccess(*KSTEP == 2 ? 13 : 67);
      if (ia != i2)
        mooseError("Mismatching pointers");

      Moose::out << *LOP << "lop " << fasum << ' ' << SMAFloatArraySize(1) << ' ' << iasum << ' '
                 << SMAIntArraySize(67) << '\n';
    }

    // beginnging of the timestep
    case 1:
    {
      Real * fa = SMAFloatArrayAccess(1);
      for (unsigned int i = 0; i < SMAFloatArraySize(1); ++i)
        fa[i] += i + *KINC;

      int * ia = SMAIntArrayAccess(1);
      for (unsigned int i = 0; i < SMAIntArraySize(67); ++i)
        ia[i] += i + *KINC;
    }

    // end of the simulation
    case 3:
    {
    }
  }
}
