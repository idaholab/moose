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

#ifndef MARKERWAREHOUSE_H
#define MARKERWAREHOUSE_H

#include <vector>
#include <map>
#include <set>

#include "Warehouse.h"

class Marker;

/**
 * Holds Markers and provides some services
 */
class MarkerWarehouse : public Warehouse<Marker>
{
public:
  MarkerWarehouse();
  virtual ~MarkerWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void markerSetup();

  /**
   * Get the list of all active Markers
   * @return The list of all active Markers
   */
  const std::vector<Marker *> & active() const { return _active_markers; }

  /**
   * Get the list of all active Markers for a variable
   */
  // const std::vector<Marker *> & activeVar(unsigned int var) { return _active_var_Markers[var]; }

  /**
   * Add a Markers
   * @param Marker Marker being added
   * @param block_ids Set of active domain where the Marker is defined
   */
  void addMarker(MooseSharedPointer<Marker> marker, std::vector<SubdomainID> & block_ids);

  /**
   * Update the list of active Markers
   * @param subdomain_id Domain ID
   */
  void updateActiveMarkers(unsigned int subdomain_id);


protected:
  ///@{
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<Marker> > _all_ptrs;
  ///@}

  /// Markers active on a block and in specified time
  std::vector<Marker *> _active_markers;
  /// Markers that live everywhere (on the whole domain)
  std::vector<Marker *> _global_markers;
  /// Markers that live on a specified block
  std::map<SubdomainID, std::vector<Marker *> > _block_markers;
};

#endif // MARKERWAREHOUSE_H
