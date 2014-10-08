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

MarkerWarehouse::MarkerWarehouse() :
    Warehouse<Marker>()
{
}

MarkerWarehouse::~MarkerWarehouse()
{
}

void
MarkerWarehouse::initialSetup()
{
  for (std::vector<MooseSharedPointer<Marker> >::const_iterator it = _all_ptrs.begin(); it != _all_ptrs.end(); ++it)
    (*it)->initialSetup();
}

void
MarkerWarehouse::timestepSetup()
{
  for (std::vector<MooseSharedPointer<Marker> >::const_iterator it = _all_ptrs.begin(); it != _all_ptrs.end(); ++it)
    (*it)->timestepSetup();
}

void
MarkerWarehouse::markerSetup()
{
  for (std::vector<MooseSharedPointer<Marker> >::const_iterator it = _all_ptrs.begin(); it != _all_ptrs.end(); ++it)
    (*it)->markerSetup();
}

void
MarkerWarehouse::addMarker(MooseSharedPointer<Marker> marker, std::vector<SubdomainID> & block_ids)
{
  _all_ptrs.push_back(marker);
  _all_objects.push_back(marker.get());

  if (block_ids.empty())
    _global_markers.push_back(marker.get());
  else
  {
    for (std::vector<SubdomainID>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      SubdomainID blk_id = *it;
      _block_markers[blk_id].push_back(marker.get());
    }
  }
}

void
MarkerWarehouse::updateActiveMarkers(unsigned int subdomain_id)
{
  _active_markers.clear();

  // add Markers that live everywhere
  for (std::vector<Marker *>::const_iterator it = _global_markers.begin(); it != _global_markers.end(); ++it)
  {
    Marker * marker = *it;
    if (marker->isActive())
      _active_markers.push_back(marker);
  }

  // then Markers that live on a specified block
  for (std::vector<Marker *>::const_iterator it = _block_markers[subdomain_id].begin(); it != _block_markers[subdomain_id].end(); ++it)
  {
    Marker * marker = *it;
    if (marker->isActive())
      _active_markers.push_back(marker);
  }

  DependencyResolverInterface::sort(_active_markers.begin(), _active_markers.end());
}
