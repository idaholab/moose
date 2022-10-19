//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceMaterial.h"
#include "TwoMaterialPropertyInterface.h"

template const ADMaterialProperty<Real> &
InterfaceMaterial::getNeighborADMaterialProperty(const std::string & name);
template const ADMaterialProperty<Real> &
InterfaceMaterial::getNeighborADMaterialPropertyByName(const std::string & name);
template const ADMaterialProperty<Real> &
TwoMaterialPropertyInterface::getNeighborADMaterialPropertyByName(const std::string & name);
