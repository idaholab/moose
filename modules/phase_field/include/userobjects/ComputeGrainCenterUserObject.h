/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEGRAINCENTERUSEROBJECT_H
#define COMPUTEGRAINCENTERUSEROBJECT_H

#include "ElementUserObject.h"

// Forward Declarations
class ComputeGrainCenterUserObject;

template <>
InputParameters validParams<ComputeGrainCenterUserObject>();

/**
 * This UserObject computes a volumes and centers of grains.
 */
class ComputeGrainCenterUserObject : public ElementUserObject
{
public:
  ComputeGrainCenterUserObject(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  const std::vector<Real> & getGrainVolumes() const;
  const std::vector<Point> & getGrainCenters() const;

protected:
  unsigned int _qp;
  unsigned int _ncrys;
  std::vector<const VariableValue *> _vals;
  unsigned int _ncomp;
  ///@{ storing volumes and centers of all the grains
  std::vector<Real> _grain_data;
  std::vector<Real> _grain_volumes;
  std::vector<Point> _grain_centers;
  ///@}
};

#endif // COMPUTEGRAINCENTERUSEROBJECT_H
