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

#include "TransferWarehouse.h"

#include "Transfer.h"
#include "MultiAppTransfer.h"
#include "MooseError.h"

TransferWarehouse::TransferWarehouse()
{
}

TransferWarehouse::~TransferWarehouse()
{
}

void
TransferWarehouse::addTransfer(MooseSharedPointer<Transfer> transfer)
{
  _all_ptrs.push_back(transfer);

  _all_objects.push_back(transfer.get());

  MooseSharedPointer<MultiAppTransfer> multi_app_transfer = MooseSharedNamespace::dynamic_pointer_cast<MultiAppTransfer>(transfer);

  if (multi_app_transfer.get())
    _multi_app_transfers.push_back(multi_app_transfer.get());
}

bool
TransferWarehouse::hasTransfer(const std::string & transfer_name) const
{
  for (std::vector<Transfer *>::const_iterator i = _all_objects.begin(); i != _all_objects.end(); ++i)
    if ((*i)->name() == transfer_name)
      return true;

  return false;
}

Transfer *
TransferWarehouse::getTransfer(const std::string & transfer_name) const
{
  for (std::vector<Transfer *>::const_iterator i = _all_objects.begin(); i != _all_objects.end(); ++i)
    if ((*i)->name() == transfer_name)
      return *i;

  mooseError("Unknown Transfer: " << transfer_name);
}

void
TransferWarehouse::initialSetup()
{
  for (std::vector<Transfer *>::iterator i = _all_objects.begin(); i != _all_objects.end(); ++i)
    (*i)->initialSetup();
}
