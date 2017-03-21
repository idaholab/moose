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

#ifndef SIDESETSAROUNDSUBDOMAIN_H
#define SIDESETSAROUNDSUBDOMAIN_H

#include "AddSideSetsBase.h" // needed for _fe_face, if restricting using normals
#include "BlockRestrictable.h"

class SideSetsAroundSubdomain;

template <>
InputParameters validParams<SideSetsAroundSubdomain>();

/**
 * Adds the faces on the boundary of given block
 * to the sidesets specified by "boundary"
 * Optionally, only adds faces that have a normal
 * equal to specified normal up to a tolerance
 */
class SideSetsAroundSubdomain : public AddSideSetsBase, public BlockRestrictable
{
public:
  SideSetsAroundSubdomain(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  virtual void modify() override;

  /// names of the sidesets to which the faces will be added
  std::vector<BoundaryName> _boundary_names;

  /// true if only faces close to "normal" will be added
  bool _using_normal;

  /**
   * if normal is specified, then faces are only added
   * if face_normal.normal_hat <= 1 - normal_tol
   * where normal_hat = _normal/|_normal|
   */
  Real _normal_tol;

  /// if specified, then faces are only added if their normal is close to this
  Point _normal;
};

#endif /* SIDESETSAROUNDSUBDOMAIN_H */
