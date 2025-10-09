//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEHydrostaticCorrectionDGKernel.h"
#include "MooseVariable.h"

registerMooseObject("ShallowWaterApp", SWEHydrostaticCorrectionDGKernel);

InputParameters
SWEHydrostaticCorrectionDGKernel::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addClassDescription("Hydrostatic correction for SWE momentum to preserve lake-at-rest.");
  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addRequiredCoupledVar("hu", "Conserved variable: h*u");
  params.addRequiredCoupledVar("hv", "Conserved variable: h*v");
  params.addRequiredCoupledVar("b_var", "Cell-constant bathymetry variable to use at faces");
  params.addRequiredCoupledVar("gravity", "Scalar gravity field g");
  return params;
}

SWEHydrostaticCorrectionDGKernel::SWEHydrostaticCorrectionDGKernel(
    const InputParameters & parameters)
  : DGKernel(parameters),
    _h1(getMaterialProperty<Real>("h")),
    _h2(getNeighborMaterialProperty<Real>("h")),
    _b1_var(coupledValue("b_var")),
    _b2_var(coupledNeighborValue("b_var")),
    _h_var(coupled("h")),
    _hu_var(coupled("hu")),
    _hv_var(coupled("hv")),
    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number())),
    _g(coupledValue("gravity"))
{
}

std::map<unsigned int, unsigned int>
SWEHydrostaticCorrectionDGKernel::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::make_pair(_h_var, 0));
  jmap.insert(std::make_pair(_hu_var, 1));
  jmap.insert(std::make_pair(_hv_var, 2));
  return jmap;
}

Real
SWEHydrostaticCorrectionDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  // Only momentum equations get correction
  if (_equation_index == 0)
    return 0.0;

  // Hydrostatic reconstruction on the face
  const Real hL = std::max(_h1[_qp], 0.0);
  const Real hR = std::max(_h2[_qp], 0.0);
  const Real bL = _b1_var[_qp];
  const Real bR = _b2_var[_qp];
  const Real etaL = hL + bL;
  const Real etaR = hR + bR;
  const Real bstar = std::max(bL, bR);
  const Real hLstar = std::max(0.0, etaL - bstar);
  const Real hRstar = std::max(0.0, etaR - bstar);

  const Real psiL = 0.5 * _g[_qp] * (hL * hL - hLstar * hLstar);
  const Real psiR = 0.5 * _g[_qp] * (hR * hR - hRstar * hRstar);

  const Real nx = _normals[_qp](0);
  const Real ny = _normals[_qp](1);
  const Real ncomp = (_equation_index == 1 ? nx : ny);

  switch (type)
  {
    case Moose::Element:
      return psiL * ncomp * _test[_i][_qp];
    case Moose::Neighbor:
      return -psiR * ncomp * _test_neighbor[_i][_qp];
  }
  return 0.0;
}

Real
SWEHydrostaticCorrectionDGKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  return computeQpOffDiagJacobian(type, _var.number());
}

Real
SWEHydrostaticCorrectionDGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                           unsigned int jvar)
{
  // Only momentum equations get correction; derivative only w.r.t. h
  if (_equation_index == 0)
    return 0.0;

  const Real nx = _normals[_qp](0);
  const Real ny = _normals[_qp](1);
  const Real ncomp = (_equation_index == 1 ? nx : ny);

  const bool wrt_h_left = (jvar == _h_var);

  // Left side derivative
  if (type == Moose::ElementElement)
  {
    if (wrt_h_left)
    {
      const Real hL = std::max(_h1[_qp], 0.0);
      const Real bL = _b1_var[_qp];
      const Real bR = _b2_var[_qp];
      const Real bstar = std::max(bL, bR);
      const Real etaL = hL + bL;
      const Real hLstar = std::max(0.0, etaL - bstar);
      // Approximate d/dh [0.5*g*(h^2 - h*^2)] ~ g*(h - h*) when h*>0, else g*h
      const Real dpsi_dh = (hLstar > 0.0 ? _g[_qp] * (hL - hLstar) : _g[_qp] * hL);
      return dpsi_dh * ncomp * _phi[_j][_qp] * _test[_i][_qp];
    }
    else
      return 0.0;
  }

  // Left wrt right or Right wrt left: no coupling in this approximation
  if (type == Moose::ElementNeighbor || type == Moose::NeighborElement)
    return 0.0;

  // NeighborNeighbor derivative w.r.t neighbor h
  if (type == Moose::NeighborNeighbor)
  {
    if (jvar == _h_var)
    {
      const Real hR = std::max(_h2[_qp], 0.0);
      const Real bL = _b1_var[_qp];
      const Real bR = _b2_var[_qp];
      const Real bstar = std::max(bL, bR);
      const Real etaR = hR + bR;
      const Real hRstar = std::max(0.0, etaR - bstar);
      const Real dpsi_dh = (hRstar > 0.0 ? _g[_qp] * (hR - hRstar) : _g[_qp] * hR);
      return -dpsi_dh * ncomp * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
    }
    else
      return 0.0;
  }

  return 0.0;
}
