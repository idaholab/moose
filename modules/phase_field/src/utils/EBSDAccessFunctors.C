//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
