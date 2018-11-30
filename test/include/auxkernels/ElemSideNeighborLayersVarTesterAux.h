//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMSIDENEIGHBORLAYERSVARTESTERAUX_H
#define ELEMSIDENEIGHBORLAYERSVARTESTERAUX_H

#include "AuxKernel.h"

// Forward Declarations
class ElemSideNeighborLayersVarTesterAux;
class ElemSideNeighborLayersVarTester;

template <>
InputParameters validParams<ElemSideNeighborLayersVarTesterAux>();

/**
 * This AuxKernel retrieves values from a ElemSideNeighborLayersVarTester
 */
class ElemSideNeighborLayersVarTesterAux : public AuxKernel
{
public:
  ElemSideNeighborLayersVarTesterAux(const InputParameters & params);

protected:
  virtual Real computeValue() override;

  const ElemSideNeighborLayersVarTester & _uo;
};

#endif // ELEMSIDENEIGHBORLAYERSVARTESTERAUX_H
