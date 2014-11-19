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

#include "IndicatorWarehouse.h"
#include "Indicator.h"
#include "InternalSideIndicator.h"

IndicatorWarehouse::IndicatorWarehouse() :
    Warehouse<Indicator>()
{
}

IndicatorWarehouse::~IndicatorWarehouse()
{
}

void
IndicatorWarehouse::initialSetup()
{
  for (std::vector<MooseSharedPointer<Indicator> >::const_iterator it = _all_ptrs.begin(); it != _all_ptrs.end(); ++it)
    (*it)->initialSetup();
}

void
IndicatorWarehouse::timestepSetup()
{
  for (std::vector<MooseSharedPointer<Indicator> >::const_iterator it = _all_ptrs.begin(); it != _all_ptrs.end(); ++it)
    (*it)->timestepSetup();
}

void
IndicatorWarehouse::IndicatorSetup()
{
  for (std::vector<MooseSharedPointer<Indicator> >::const_iterator it = _all_ptrs.begin(); it != _all_ptrs.end(); ++it)
    (*it)->IndicatorSetup();
}

void
IndicatorWarehouse::addIndicator(MooseSharedPointer<Indicator> indicator, std::vector<SubdomainID> & block_ids)
{
  _all_ptrs.push_back(indicator);
  _all_objects.push_back(indicator.get());

  bool internal_side_indicator = false;

  if (MooseSharedNamespace::dynamic_pointer_cast<InternalSideIndicator>(indicator).get())
    internal_side_indicator = true;

  if (block_ids.empty())
  {
    if (internal_side_indicator)
      _global_internal_side_indicators.push_back(indicator.get());
    else
      _global_indicators.push_back(indicator.get());
  }
  else
  {
    for (std::vector<SubdomainID>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      SubdomainID blk_id = *it;

      if (internal_side_indicator)
        _block_internal_side_indicators[blk_id].push_back(indicator.get());
      else
        _block_indicators[blk_id].push_back(indicator.get());
    }
  }
}

void
IndicatorWarehouse::updateActiveIndicators(unsigned int subdomain_id)
{
  _active_indicators.clear();
  _active_internal_side_indicators.clear();

  // add Indicators that live everywhere
  for (std::vector<Indicator *>::const_iterator it = _global_indicators.begin(); it != _global_indicators.end(); ++it)
  {
    Indicator * indicator = *it;
    if (indicator->isActive())
      _active_indicators.push_back(indicator);
  }

  // then Indicators that live on a specified block
  for (std::vector<Indicator *>::const_iterator it = _block_indicators[subdomain_id].begin(); it != _block_indicators[subdomain_id].end(); ++it)
  {
    Indicator * indicator = *it;
    if (indicator->isActive())
      _active_indicators.push_back(indicator);
  }

  // add Internal_Side_Indicators that live everywhere
  for (std::vector<Indicator *>::const_iterator it = _global_internal_side_indicators.begin(); it != _global_internal_side_indicators.end(); ++it)
  {
    Indicator * indicator = *it;
    if (indicator->isActive())
      _active_internal_side_indicators.push_back(indicator);
  }

  // then Internal_Side_Indicators that live on a specified block
  for (std::vector<Indicator *>::const_iterator it = _block_internal_side_indicators[subdomain_id].begin(); it != _block_internal_side_indicators[subdomain_id].end(); ++it)
  {
    Indicator * indicator = *it;
    if (indicator->isActive())
      _active_internal_side_indicators.push_back(indicator);
  }
}
