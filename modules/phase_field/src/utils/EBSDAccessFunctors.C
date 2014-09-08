#include "EBSDAccessFunctors.h"

MooseEnum
EBSDAccessFunctors::getPointDataFieldType()
{
  return MooseEnum("phi1 phi phi2 grain phase symmetry op");
}

MooseEnum
EBSDAccessFunctors::getAvgDataFieldType()
{
  return MooseEnum("phi1 phi phi2 phase symmetry");
}

EBSDAccessFunctors::EBSDPointDataFunctor *
EBSDAccessFunctors::getPointDataAccessFunctor(const MooseEnum & field_name)
{
  switch (field_name)
  {
    case 0: // phi1
      return new EBSDPointDataPhi1();
    case 1: // phi
      return new EBSDPointDataPhi();
    case 2: // phi2
      return new EBSDPointDataPhi2();
    case 3: // grain
      return new EBSDPointDataGrain();
    case 4: // phase
      return new EBSDPointDataPhase();
    case 5: // symmetry
      return new EBSDPointDataSymmetry();
    case 6: // op
      return new EBSDPointDataOp();
  }

  mooseError("Error:  Please input supported EBSD_param");
}

EBSDAccessFunctors::EBSDAvgDataFunctor *
EBSDAccessFunctors::getAvgDataAccessFunctor(const MooseEnum & field_name)
{
  switch (field_name)
  {
    case 0: // phi1
      return new EBSDAvgDataPhi1();
    case 1: // phi
      return new EBSDAvgDataPhi();
    case 2: // phi2
      return new EBSDAvgDataPhi2();
    case 3: // phase
      return new EBSDAvgDataPhase();
    case 4: // symmetry
      return new EBSDAvgDataSymmetry();
  }

  mooseError("Error:  Please input supported EBSD_param");
}
