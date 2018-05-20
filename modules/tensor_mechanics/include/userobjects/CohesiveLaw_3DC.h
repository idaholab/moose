//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COHESIVELAW_3DC_H
#define COHESIVELAW_3DC_H

#include "TractionSeparationUOBase.h"

class CohesiveLaw_3DC;

template <>
InputParameters validParams<CohesiveLaw_3DC>();

/**
Traction sepration law basic user object
 */
class CohesiveLaw_3DC : public TractionSeparationUOBase
{
public:
  CohesiveLaw_3DC(const InputParameters & parameters);

  virtual void computeTractionLocal(unsigned int qp,
                                    RealVectorValue & TractionLocal) const override;
  virtual void
  computeTractionSpatialDerivativeLocal(unsigned int qp,
                                        RankTwoTensor & TractionDerivativeLocal) const override;

protected:
  // cohesive law paramters
  const std::vector<Real> _deltaU0;
  const std::vector<Real> _maxAllowableTraction;
};

#endif // COHESIVELAW_3DC_H
