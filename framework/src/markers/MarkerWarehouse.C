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

#include "MarkerWarehouse.h"
#include "Marker.h"

MarkerWarehouse::MarkerWarehouse()
{
}

MarkerWarehouse::~MarkerWarehouse()
{
  for (std::vector<Marker *>::const_iterator i = _all_markers.begin(); i != _all_markers.end(); ++i)
    delete *i;

}

void
MarkerWarehouse::initialSetup()
{
  for (std::vector<Marker *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->initialSetup();
}

void
MarkerWarehouse::timestepSetup()
{
  for (std::vector<Marker *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->timestepSetup();
}

void
MarkerWarehouse::markerSetup()
{
  for (std::vector<Marker *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->markerSetup();
}

void
MarkerWarehouse::addMarker(Marker *marker, std::vector<SubdomainID> & block_ids)
{
  _all_markers.push_back(marker);

  if (block_ids.empty())
  {
    _global_markers.push_back(marker);
  }
  else
  {
    for (std::vector<SubdomainID>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      SubdomainID blk_id = *it;
      _block_markers[blk_id].push_back(marker);
    }
  }
}

void
MarkerWarehouse::updateActiveMarkers(unsigned int subdomain_id)
{
  _active_markers.clear();
  //_active_var_Markers.clear();

  // add Markers that live everywhere
  for (std::vector<Marker *>::const_iterator it = _global_markers.begin(); it != _global_markers.end(); ++it)
  {
    Marker * marker = *it;
    if (marker->isActive())
    {
      _active_markers.push_back(marker);
      //_active_var_Markers[Marker->variable().number()].push_back(Marker);
    }
  }

  // then Markers that live on a specified block
  for (std::vector<Marker *>::const_iterator it = _block_markers[subdomain_id].begin(); it != _block_markers[subdomain_id].end(); ++it)
  {
    Marker * marker = *it;
    if (marker->isActive())
    {
      _active_markers.push_back(marker);
      //_active_var_Markers[Marker->variable().number()].push_back(Marker);
    }
  }
}
