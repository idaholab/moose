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

IndicatorWarehouse::IndicatorWarehouse()
{
}

IndicatorWarehouse::~IndicatorWarehouse()
{
  for (std::vector<Indicator *>::const_iterator i = _all_indicators.begin(); i != _all_indicators.end(); ++i)
    delete *i;

}

void
IndicatorWarehouse::initialSetup()
{
  for (std::vector<Indicator *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->initialSetup();
}

void
IndicatorWarehouse::timestepSetup()
{
  for (std::vector<Indicator *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->timestepSetup();
}

void
IndicatorWarehouse::IndicatorSetup()
{
  for (std::vector<Indicator *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->IndicatorSetup();
}

void
IndicatorWarehouse::addIndicator(Indicator *Indicator, std::vector<SubdomainID> & block_ids)
{
  _all_indicators.push_back(Indicator);

  bool internal_side_indicator = false;

  if(dynamic_cast<InternalSideIndicator*>(Indicator))
    internal_side_indicator = true;

  if (block_ids.empty())
  {
    if(internal_side_indicator)
      _global_internal_side_indicators.push_back(Indicator);
    else
      _global_indicators.push_back(Indicator);
  }
  else
  {
    for (std::vector<SubdomainID>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      SubdomainID blk_id = *it;

      if(internal_side_indicator)
        _block_internal_side_indicators[blk_id].push_back(Indicator);
      else
        _block_indicators[blk_id].push_back(Indicator);
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
    Indicator * Indicator = *it;
    if (Indicator->isActive())
    {
      _active_indicators.push_back(Indicator);
      //_active_var_indicators[Indicator->variable().number()].push_back(Indicator);
    }
  }

  // then Indicators that live on a specified block
  for (std::vector<Indicator *>::const_iterator it = _block_indicators[subdomain_id].begin(); it != _block_indicators[subdomain_id].end(); ++it)
  {
    Indicator * Indicator = *it;
    if (Indicator->isActive())
    {
      _active_indicators.push_back(Indicator);
      //_active_var_indicators[Indicator->variable().number()].push_back(Indicator);
    }
  }




  // add Internal_Side_Indicators that live everywhere
  for (std::vector<Indicator *>::const_iterator it = _global_internal_side_indicators.begin(); it != _global_internal_side_indicators.end(); ++it)
  {
    Indicator * Indicator = *it;
    if (Indicator->isActive())
    {
      _active_internal_side_indicators.push_back(Indicator);
      //_active_var_internal_side_indicators[Indicator->variable().number()].push_back(Indicator);
    }
  }

  // then Internal_Side_Indicators that live on a specified block
  for (std::vector<Indicator *>::const_iterator it = _block_internal_side_indicators[subdomain_id].begin(); it != _block_internal_side_indicators[subdomain_id].end(); ++it)
  {
    Indicator * Indicator = *it;
    if (Indicator->isActive())
    {
      _active_internal_side_indicators.push_back(Indicator);
      //_active_var_internal_side_indicators[Indicator->variable().number()].push_back(Indicator);
    }
  }
}
