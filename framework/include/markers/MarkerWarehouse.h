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

#include "Moose.h"
#include "MooseTypes.h"

class Marker;

/**
 * Holds Markers and provides some services
 */
class MarkerWarehouse
{
public:
  MarkerWarehouse();
  virtual ~MarkerWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void markerSetup();

  /**
   * Get list of all Markers
   * @return The list of all active Markers
   */
  const std::vector<Marker *> & all() { return _all_markers; }

  /**
   * Get the list of all active Markers
   * @return The list of all active Markers
   */
  const std::vector<Marker *> & active() { return _active_markers; }

  /**
   * Get the list of all active Markers for a variable
   * @param var The variable number
   * @return The list of all active Markers
   */
  //const std::vector<Marker *> & activeVar(unsigned int var) { return _active_var_Markers[var]; }

  /**
   * Add a Markers
   * @param Marker Marker being added
   * @param block_ids Set of active domain where the Marker is defined
   */
  void addMarker(Marker *Marker, std::vector<SubdomainID> & block_ids);

  /**
   * Update the list of active Markers
   * @param t Time
   * @param dt Time step size
   * @param subdomain_id Domain ID
   */
  void updateActiveMarkers(unsigned int subdomain_id);


protected:
  /// Markers active on a block and in specified time
  std::vector<Marker *> _active_markers;
  /// Markers active on a block and in specified time per variable
//  std::map<SubdomainID, std::vector<Marker *> > _active_var_Markers;
  /// All instances of Markers
  std::vector<Marker *> _all_markers;
  /// Markers that live everywhere (on the whole domain)
  std::vector<Marker *> _global_markers;
  /// Markers that live on a specified block
  std::map<SubdomainID, std::vector<Marker *> > _block_markers;
};

#endif // MARKERWAREHOUSE_H
