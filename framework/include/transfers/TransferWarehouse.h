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

#ifndef TRANSFERWAREHOUSE_H
#define TRANSFERWAREHOUSE_H

#include <vector>
#include <map>
#include <set>

#include "MooseTypes.h"

class Transfer;
class MultiAppTransfer;

/**
 * Holds Transfers and provides some services
 */
class TransferWarehouse
{
public:
  TransferWarehouse();
  virtual ~TransferWarehouse();

  /**
   * Get list of all Transfers
   * @return The list of all active Transfers
   */
  const std::vector<Transfer *> & all() { return _all_transfers; }

  /**
   * Get list of all MultiAppTransfers
   * @return The list of all active MultiAppTransfers
   */
  const std::vector<MultiAppTransfer *> & multiAppTransfers() { return _multi_app_transfers; }

  /**
   * Add a Transfers
   * @param transfer Transfer being added
   */
  void addTransfer(Transfer * transfer);

  /**
   * Whether or not this warehouse has a Transfer named transfer_name
   * @param transfer_name The name of the Transfer we're looking for
   * @return True if that Transfer exists False otherwise
   */
  bool hasTransfer(const std::string & transfer_name);

  /**
   * Get a Transfer by name.  Will error if the Transfer doesn't exist in this Warehouse.
   * @param transfer_name The name of the Transfer to get.
   * @return A pointer to the Transfer
   */
  Transfer * getTransfer(const std::string & transfer_name);

protected:
  std::vector<Transfer *> _all_transfers;
  std::vector<MultiAppTransfer *> _multi_app_transfers;
};

#endif // TRANSFERWAREHOUSE_H
