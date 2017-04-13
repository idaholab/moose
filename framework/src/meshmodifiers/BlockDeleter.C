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

#include "BlockDeleter.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<BlockDeleter>()
{
  InputParameters params = validParams<ElementDeleterBase>();
  params.addRequiredParam<SubdomainID>("block_id", "The block to be deleted");
  return params;
}

BlockDeleter::BlockDeleter(const InputParameters & parameters)
  : ElementDeleterBase(parameters), _block_id(getParam<SubdomainID>("block_id"))
{
}

bool
BlockDeleter::shouldDelete(const Elem * elem)
{
  return elem->subdomain_id() == _block_id;
}
