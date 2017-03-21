/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DISCRETENUCLEATIONINSERTER_H
#define DISCRETENUCLEATIONINSERTER_H

#include "ElementUserObject.h"

class DiscreteNucleationInserter;

template <>
InputParameters validParams<DiscreteNucleationInserter>();

/**
 * This UserObject manages the insertion and expiration of nuclei in the simulation
 * domain it manages a list of nuclei with their insertion times and their center
 * positions. A DiscreteNucleationMap is needed to enable the DiscreteNucleation
 * material to look up if a nucleus is present at a given element/qp.
 */
class DiscreteNucleationInserter : public ElementUserObject
{
public:
  DiscreteNucleationInserter(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  /// A nucleus has an expiration time and a location
  typedef std::pair<Real, Point> NucleusLocation;

  /// Every MPI task should keep a full list of nuclei (in case they cross domains with their finite radii)
  typedef std::vector<NucleusLocation> NucleusList;

  const NucleusList & getNucleusList() const { return _global_nucleus_list; }
  bool isMapUpdateRequired() const { return _changes_made > 0; }

protected:
  /// Nucleation rate density (should be a material property implementing nucleation theory)
  const MaterialProperty<Real> & _probability;

  /// Duration of time each nucleus is kept active after insertion
  Real _hold_time;

  /// count the number of nucleus deletions and insertions
  unsigned int _changes_made;

  /// the global list of all nuclei over all processors
  NucleusList & _global_nucleus_list;

  /// the local nucleus list of nuclei centered in the domain of the current processor
  NucleusList & _local_nucleus_list;

  /// insert test location
  bool _insert_test;
};

#endif // DISCRETENUCLEATIONINSERTER_H
