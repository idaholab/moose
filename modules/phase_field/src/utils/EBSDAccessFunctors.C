#include "EBSDAccessFunctors.h"

MooseEnum
EBSDAccessFunctors::getPointDataFieldType()
{
  return MooseEnum("phi1 phi phi2 feature_id phase symmetry", "", true);
}

MooseEnum
EBSDAccessFunctors::getAvgDataFieldType()
{
  return MooseEnum("phi1 phi phi2 phase symmetry local_id feature_id", "", true);
}
