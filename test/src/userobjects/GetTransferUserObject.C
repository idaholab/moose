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

#include "GetTransferUserObject.h"
#include "libmesh/quadrature.h"
#include "MultiAppTransfer.h"

template <>
InputParameters
validParams<GetTransferUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
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
