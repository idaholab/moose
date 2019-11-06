//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GetTransferUserObject.h"
#include "libmesh/quadrature.h"
#include "MultiAppTransfer.h"

registerMooseObject("MooseTestApp", GetTransferUserObject);

InputParameters
GetTransferUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

GetTransferUserObject::GetTransferUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

void
GetTransferUserObject::execute()
{
}

void
GetTransferUserObject::initialize()
{
  std::vector<std::shared_ptr<Transfer>> transfers =
      _fe_problem.getTransfers(EXEC_TIMESTEP_END, MultiAppTransfer::TO_MULTIAPP);
  if (transfers.size() != 2)
    mooseError("Number of transfers in GetTransferUserObject incorrect");
  for (auto & t : transfers)
  {
    std::shared_ptr<MultiAppTransfer> mt = std::dynamic_pointer_cast<MultiAppTransfer>(t);
    if (!mt)
      mooseError("Transfer ", t->name(), " is not a MultiAppTransfer");
  }
}

void
GetTransferUserObject::finalize()
{
}
