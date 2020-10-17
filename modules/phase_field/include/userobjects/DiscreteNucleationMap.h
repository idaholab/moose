//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "DiscreteNucleationInserterBase.h"

/**
 * This UserObject maintains a per QP map that indicates if a nucleus is
 * present or not. It effectively performs a spatial hashing of the list maintained
 * by the DiscreteNucleationInserter (and allows for spatially extended nuclei)
 */
class DiscreteNucleationMap : public ElementUserObject
{
public:
  static InputParameters validParams();

  DiscreteNucleationMap(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize() {}

  virtual void meshChanged();

  const std::vector<Real> & nuclei(const Elem *) const;

  const DiscreteNucleationInserterBase & getInserter() const { return _inserter; }

  Real getWidth() const { return _int_width; }
  Real getPeriodic() const { return _periodic; }

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
  const DiscreteNucleationInserterBase & _inserter;

  /// variable number to use for minPeriodicDistance calls (i.e. use the periodicity of this variable)
  int _periodic;

  /// Nucleus interface width
  const Real _int_width;

  /// list of nuclei maintained bu the inserter object
  const DiscreteNucleationInserterBase::NucleusList & _nucleus_list;

  ///@{ Per element list with 0/1 flags indicating the presence of a nucleus
  using NucleusMap = std::unordered_map<dof_id_type, std::vector<Real>>;
  NucleusMap _nucleus_map;
  ///@}
};
