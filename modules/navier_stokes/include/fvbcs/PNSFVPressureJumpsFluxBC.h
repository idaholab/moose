//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFluxBC.h"

class PNSFVPressureJumpsFluxBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  PNSFVPressureJumpsFluxBC(const InputParameters & params);
  void computeResidual(const FaceInfo & fi) override;
  void computeJacobian(const FaceInfo & fi) override;

protected:
  ADReal computeQpResidual() override;

  const MaterialProperty<Real> & _eps_elem;
  const MaterialProperty<Real> & _eps_neighbor;
  const ADMaterialProperty<Real> & _pressure_elem;
  const ADMaterialProperty<Real> & _pressure_neighbor;
  const unsigned int _index;
  const bool _has_p_boundary;
  const Function & _p_boundary;
  const ADReal * _pressure_elem_qp;
  const ADReal * _pressure_neighbor_qp;
};
