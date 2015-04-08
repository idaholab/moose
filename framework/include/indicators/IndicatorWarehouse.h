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

#ifndef INDICATORWAREHOUSE_H
#define INDICATORWAREHOUSE_H

#include <vector>
#include <map>
#include <set>

#include "Warehouse.h"

class Indicator;

/**
 * Holds Indicators and provides some services
 */
class IndicatorWarehouse : public Warehouse<Indicator>
{
public:
  IndicatorWarehouse();
  virtual ~IndicatorWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void IndicatorSetup();

  /**
   * Get the list of all active Indicators
   * @return The list of all active Indicators
   */
  const std::vector<Indicator *> & active() const { return _active_indicators; }

  /**
   * Get the list of all active Indicators
   * @return The list of all active InternalSideIndicators
   */
  const std::vector<Indicator *> & activeInternalSideIndicators() const { return _active_internal_side_indicators; }

  /**
   * Add a Indicators
   * @param Indicator Indicator being added
   * @param block_ids Set of active domain where the Indicator is defined
   */
  void addIndicator(MooseSharedPointer<Indicator> indicator, std::vector<SubdomainID> & block_ids);

  /**
   * Update the list of active Indicators
   * @param subdomain_id Domain ID
   */
  void updateActiveIndicators(unsigned int subdomain_id);


protected:
  ///@{
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<Indicator> > _all_ptrs;
  ///@}

  /// Indicators active on a block and in specified time
  std::vector<Indicator *> _active_indicators;

  std::vector<Indicator *> _active_internal_side_indicators;

  /// Indicators that live everywhere (on the whole domain)
  std::vector<Indicator *> _global_indicators;

  /// Indicators that live on a specified block
  std::map<SubdomainID, std::vector<Indicator *> > _block_indicators;

  /// Indicators that live everywhere (on the whole domain)
  std::vector<Indicator *> _global_internal_side_indicators;

  /// Indicators that live on a specified block
  std::map<SubdomainID, std::vector<Indicator *> > _block_internal_side_indicators;
};

#endif // INDICATORWAREHOUSE_H
