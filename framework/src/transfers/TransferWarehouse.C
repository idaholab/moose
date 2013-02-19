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
  for (std::vector<Transfer *>::const_iterator i = _all_transfers.begin(); i != _all_transfers.end(); ++i)
    delete *i;

}

void
TransferWarehouse::addTransfer(Transfer * transfer)
{
  _all_transfers.push_back(transfer);

  MultiAppTransfer * multi_app_transfer = dynamic_cast<MultiAppTransfer *>(transfer);

  if(multi_app_transfer)
    _multi_app_transfers.push_back(multi_app_transfer);
}

bool
TransferWarehouse::hasTransfer(const std::string & transfer_name)
{
  for (std::vector<Transfer *>::const_iterator i = _all_transfers.begin(); i != _all_transfers.end(); ++i)
    if((*i)->name() == transfer_name)
      return true;

  return false;
}

Transfer *
TransferWarehouse::getTransfer(const std::string & transfer_name)
{
  for (std::vector<Transfer *>::const_iterator i = _all_transfers.begin(); i != _all_transfers.end(); ++i)
    if((*i)->name() == transfer_name)
      return *i;

  mooseError("Unknown Transfer: " << transfer_name);
}

