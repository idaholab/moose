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

#ifndef REASSIGNSUBDOMAINIDS_H
#define REASSIGNSUBDOMAINIDS_H

// MOOSE includes
#include "MeshModifier.h"

// Forward declerations
class ReassignSubdomainIDs;

template<>
InputParameters validParams<ReassignSubdomainIDs>();

/**
 * MeshModifier for changing Subdomain IDs
 */
class ReassignSubdomainIDs : public MeshModifier
{
public:

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  ReassignSubdomainIDs(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~ReassignSubdomainIDs();

  /**
   * Perform the actual element subdomain ID re-assignment
   */
  virtual void modify();

protected:
  /// The original block IDs
  const std::vector<SubdomainID> & _block;

  /// The new block IDs
  const std::vector<SubdomainID> & _new_block;

  /// Map of original->new
  std::map<SubdomainID, SubdomainID> _block_map;
};

#endif //REASSIGNSUBDOMAINIDS_H
