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

#include "Moose.h"
#include "MooseTypes.h"

class Indicator;

/**
 * Holds Indicators and provides some services
 */
class IndicatorWarehouse
{
public:
  IndicatorWarehouse();
  virtual ~IndicatorWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void IndicatorSetup();

  /**
   * Get list of all Indicators
   * @return The list of all active Indicators
   */
  const std::vector<Indicator *> & all() { return _all_Indicators; }

  /**
   * Get the list of all active Indicators
   * @return The list of all active Indicators
   */
  const std::vector<Indicator *> & active() { return _active_Indicators; }

  /**
   * Get the list of all active Indicators for a variable
   * @param var The variable number
   * @return The list of all active Indicators
   */
  //const std::vector<Indicator *> & activeVar(unsigned int var) { return _active_var_Indicators[var]; }

  /**
   * Add a Indicators
   * @param Indicator Indicator being added
   * @param block_ids Set of active domain where the Indicator is defined
   */
  void addIndicator(Indicator *Indicator, std::vector<SubdomainID> & block_ids);

  /**
   * Update the list of active Indicators
   * @param t Time
   * @param dt Time step size
   * @param subdomain_id Domain ID
   */
  void updateActiveIndicators(unsigned int subdomain_id);


protected:
  /// Indicators active on a block and in specified time
  std::vector<Indicator *> _active_Indicators;
  /// Indicators active on a block and in specified time per variable
//  std::map<SubdomainID, std::vector<Indicator *> > _active_var_Indicators;
  /// All instances of Indicators
  std::vector<Indicator *> _all_Indicators;
  /// Indicators that live everywhere (on the whole domain)
  std::vector<Indicator *> _global_Indicators;
  /// Indicators that live on a specified block
  std::map<SubdomainID, std::vector<Indicator *> > _block_Indicators;

};

#endif // INDICATORWAREHOUSE_H
