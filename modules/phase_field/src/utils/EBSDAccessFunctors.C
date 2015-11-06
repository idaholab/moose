#include "EBSDAccessFunctors.h"

MooseEnum
EBSDAccessFunctors::getPointDataFieldType()
{
  return MooseEnum("phi1 phi phi2 grain phase symmetry op", "", true);
}

MooseEnum
EBSDAccessFunctors::getAvgDataFieldType()
{
  return MooseEnum("phi1 phi phi2 phase symmetry local grain", "", true);
}
