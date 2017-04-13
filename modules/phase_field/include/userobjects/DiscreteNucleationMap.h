/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DISCRETENUCLEATIONMAP_H
#define DISCRETENUCLEATIONMAP_H

#include "ElementUserObject.h"
#include "DiscreteNucleationInserter.h"

class DiscreteNucleationMap;

template <>
InputParameters validParams<DiscreteNucleationMap>();

/**
 * This UserObject maintains a per QP map that indicates if a nucleus is
 * present or not. It effectively performs a spatial hashing of the list maintained
 * by the DiscreteNucleationInserter (and allows for spatially extended nuclei)
 */
class DiscreteNucleationMap : public ElementUserObject
{
public:
  DiscreteNucleationMap(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize() {}

  virtual void meshChanged();

  const std::vector<Real> & nuclei(const Elem *) const;

protected:
  /// Did the mesh change since the last execution of this PP?
  bool _mesh_changed;

  /// Do we need to rebuild the map during this timestep?
  bool _rebuild_map;

  /// Buffer for building the per QP map
  std::vector<Real> _elem_map;

  /// Dummy map for elements without nuclei
  std::vector<Real> _zero_map;

  /// UserObject that manages nucleus insertin and deletion
  const DiscreteNucleationInserter & _inserter;

  /// variable number to use for minPeriodicDistance calls (i.e. use the periodicity of this variable)
  int _periodic;

  /// Nucleus radius
  const Real _radius;

  /// Nucleus interface width
  const Real _int_width;

  /// list of nuclei maintained bu the inserter object
  const DiscreteNucleationInserter::NucleusList & _nucleus_list;

  ///@{
  /// Per element list with 0/1 flags indicating the presence of a nucleus
  using NucleusMap = std::unordered_map<dof_id_type, std::vector<Real>>;
  NucleusMap _nucleus_map;
  ///@}
};

#endif // DISCRETENUCLEATIONMAP_H
