//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMSIDENEIGHBORLAYERSGEOMTESTER_H
#define ELEMSIDENEIGHBORLAYERSGEOMTESTER_H

#include "ElemSideNeighborLayersTester.h"

// Forward Declarations
class ElemSideNeighborLayersGeomTester;

template <>
InputParameters validParams<ElemSideNeighborLayersGeomTester>();

/**
 * User object to show information about the ElemSideNeighborLayer object's "ghosting" behaviors
 */
class ElemSideNeighborLayersGeomTester : public ElemSideNeighborLayersTester
{
public:
  ElemSideNeighborLayersGeomTester(const InputParameters & parameters);
};

#endif // ELEMSIDENEIGHBORLAYERSGEOMTESTER_H
