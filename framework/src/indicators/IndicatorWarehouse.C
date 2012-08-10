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

IndicatorWarehouse::IndicatorWarehouse()
{
}

IndicatorWarehouse::~IndicatorWarehouse()
{
  for (std::vector<Indicator *>::const_iterator i = _all_Indicators.begin(); i != _all_Indicators.end(); ++i)
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
  _all_Indicators.push_back(Indicator);

  if (block_ids.empty())
  {
    _global_Indicators.push_back(Indicator);
  }
  else
  {
    for (std::vector<SubdomainID>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      SubdomainID blk_id = *it;
      _block_Indicators[blk_id].push_back(Indicator);
    }
  }
}

void
IndicatorWarehouse::updateActiveIndicators(unsigned int subdomain_id)
{
  _active_Indicators.clear();
  //_active_var_Indicators.clear();

  // add Indicators that live everywhere
  for (std::vector<Indicator *>::const_iterator it = _global_Indicators.begin(); it != _global_Indicators.end(); ++it)
  {
    Indicator * Indicator = *it;
    if (Indicator->isActive())
    {
      _active_Indicators.push_back(Indicator);
      //_active_var_Indicators[Indicator->variable().number()].push_back(Indicator);
    }
  }

  // then Indicators that live on a specified block
  for (std::vector<Indicator *>::const_iterator it = _block_Indicators[subdomain_id].begin(); it != _block_Indicators[subdomain_id].end(); ++it)
  {
    Indicator * Indicator = *it;
    if (Indicator->isActive())
    {
      _active_Indicators.push_back(Indicator);
      //_active_var_Indicators[Indicator->variable().number()].push_back(Indicator);
    }
  }
}
