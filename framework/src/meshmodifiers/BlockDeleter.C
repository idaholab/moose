//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockDeleter.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<BlockDeleter>()
{
  InputParameters params = validParams<ElementDeleterBase>();
  params.addClassDescription(
      "Mesh modifier which removes elements with the specified subdomain ID");
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
