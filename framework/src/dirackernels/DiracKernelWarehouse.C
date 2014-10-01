/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "DiracKernelWarehouse.h"
#include "DiracKernel.h"


DiracKernelWarehouse::DiracKernelWarehouse() :
    Warehouse<DiracKernel>()
{
}

DiracKernelWarehouse::~DiracKernelWarehouse()
{
}

void
DiracKernelWarehouse::initialSetup()
{
  for (unsigned int i=0; i<_all_objects.size(); i++)
    _all_objects[i]->initialSetup();
}

void
DiracKernelWarehouse::timestepSetup()
{
  for (unsigned int i=0; i<_all_objects.size(); i++)
    _all_objects[i]->timestepSetup();
}

void
DiracKernelWarehouse::residualSetup()
{
  for (unsigned int i=0; i<_all_objects.size(); i++)
    _all_objects[i]->residualSetup();
}

void
DiracKernelWarehouse::jacobianSetup()
{
  for (unsigned int i=0; i<_all_objects.size(); i++)
    _all_objects[i]->jacobianSetup();
}

void
DiracKernelWarehouse::addDiracKernel(MooseSharedPointer<DiracKernel> & kernel)
{
  _all_ptrs.push_back(kernel);
  _all_objects.push_back(kernel.get());
}
