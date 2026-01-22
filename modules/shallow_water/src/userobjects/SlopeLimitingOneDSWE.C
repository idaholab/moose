//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* Licensed under LGPL 2.1

#include "SlopeLimitingOneDSWE.h"

registerMooseObject("ShallowWaterApp", SlopeLimitingOneDSWE);

InputParameters
SlopeLimitingOneDSWE::validParams()
{
  InputParameters params = SlopeLimitingBase::validParams();
  params.addClassDescription(
      "1D MUSCL slope limiter for SWE variables h, hu, hv (minmod/MC/superbee)");
  params.addRequiredCoupledVar("h", "Cell-constant depth h");
  params.addRequiredCoupledVar("hu", "Cell-constant x-momentum hu");
  params.addRequiredCoupledVar("hv", "Cell-constant y-momentum hv");
  MooseEnum scheme("none minmod mc superbee", "mc");
  params.addParam<MooseEnum>("scheme", scheme, "TVD-type slope limiting scheme");
  return params;
}

SlopeLimitingOneDSWE::SlopeLimitingOneDSWE(const InputParameters & parameters)
  : SlopeLimitingBase(parameters),
    _hvar(getVar("h", 0)),
    _huvar(getVar("hu", 0)),
    _hvvar(getVar("hv", 0)),
    _scheme(getParam<MooseEnum>("scheme"))
{
}

std::vector<libMesh::RealGradient>
SlopeLimitingOneDSWE::limitElementSlope() const
{
  const Elem * elem = _current_elem;
  const unsigned int nside = elem->n_sides();

  // Return zero slopes if not 1D (two sides)
  std::vector<RealGradient> grad(3, RealGradient(0., 0., 0.));
  if (nside != 2)
    return grad;

  // Centroid x of central and neighbor cells
  Real xc[3] = {0.0, 0.0, 0.0};
  xc[0] = elem->vertex_average()(0);

  // Cell-average values: [center,left,right] x [h,hu,hv]
  Real U[3][3];
  for (unsigned k = 0; k < 3; ++k)
    U[0][k] = 0.0;

  // Get central cell averages (implicit/new or old depending on scheme context)
  if (_is_implicit)
  {
    U[0][0] = _hvar->getElementalValue(elem);
    U[0][1] = _huvar->getElementalValue(elem);
    U[0][2] = _hvvar->getElementalValue(elem);
  }
  else
  {
    U[0][0] = _hvar->getElementalValueOld(elem);
    U[0][1] = _huvar->getElementalValueOld(elem);
    U[0][2] = _hvvar->getElementalValueOld(elem);
  }

  // One-sided slopes sigma[side][var], side: 1=left, 2=right (in rdg AEFV conventions)
  Real sigma[3][3];
  for (unsigned s = 0; s < 3; ++s)
    for (unsigned k = 0; k < 3; ++k)
      sigma[s][k] = 0.0;

  unsigned int bflag = 0; // if any side lacks neighbor, degrade to zero slope

  // Left neighbor (assume side 0)
  if (elem->neighbor_ptr(0))
  {
    const Elem * neig = elem->neighbor_ptr(0);
    if (this->hasBlocks(neig->subdomain_id()))
    {
      xc[1] = neig->vertex_average()(0);
      if (_is_implicit)
      {
        U[1][0] = _hvar->getElementalValue(neig);
        U[1][1] = _huvar->getElementalValue(neig);
        U[1][2] = _hvvar->getElementalValue(neig);
      }
      else
      {
        U[1][0] = _hvar->getElementalValueOld(neig);
        U[1][1] = _huvar->getElementalValueOld(neig);
        U[1][2] = _hvvar->getElementalValueOld(neig);
      }
      for (unsigned k = 0; k < 3; ++k)
        sigma[1][k] = (U[0][k] - U[1][k]) / (xc[0] - xc[1]);
    }
    else
      bflag = 1;
  }
  else
    bflag = 1;

  // Right neighbor (assume side 1)
  if (elem->neighbor_ptr(1))
  {
    const Elem * neig = elem->neighbor_ptr(1);
    if (this->hasBlocks(neig->subdomain_id()))
    {
      xc[2] = neig->vertex_average()(0);
      if (_is_implicit)
      {
        U[2][0] = _hvar->getElementalValue(neig);
        U[2][1] = _huvar->getElementalValue(neig);
        U[2][2] = _hvvar->getElementalValue(neig);
      }
      else
      {
        U[2][0] = _hvar->getElementalValueOld(neig);
        U[2][1] = _huvar->getElementalValueOld(neig);
        U[2][2] = _hvvar->getElementalValueOld(neig);
      }
      for (unsigned k = 0; k < 3; ++k)
        sigma[2][k] = (U[2][k] - U[0][k]) / (xc[2] - xc[0]);
    }
    else
      bflag = 2;
  }
  else
    bflag = 2;

  // If boundary detected, keep zero slopes (first-order)
  if (bflag != 0)
    return grad;

  // Central slope
  for (unsigned k = 0; k < 3; ++k)
    sigma[0][k] = (U[2][k] - U[1][k]) / (xc[2] - xc[1]);

  // Apply limiter per variable
  switch (_scheme)
  {
    case 0: // none
      break;
    case 1: // minmod
      for (unsigned k = 0; k < 3; ++k)
      {
        if (sigma[1][k] * sigma[2][k] > 0.0)
          grad[k](0) = (std::abs(sigma[1][k]) < std::abs(sigma[2][k])) ? sigma[1][k] : sigma[2][k];
      }
      break;
    case 2: // MC
      for (unsigned k = 0; k < 3; ++k)
      {
        const Real s0 = sigma[0][k], s1 = sigma[1][k], s2 = sigma[2][k];
        if (s0 > 0.0 && s1 > 0.0 && s2 > 0.0)
          grad[k](0) = std::min(s0, 2.0 * std::min(s1, s2));
        else if (s0 < 0.0 && s1 < 0.0 && s2 < 0.0)
          grad[k](0) = std::max(s0, 2.0 * std::max(s1, s2));
      }
      break;
    case 3: // superbee
      for (unsigned k = 0; k < 3; ++k)
      {
        Real s1 = 0.0, s2 = 0.0;
        if (sigma[2][k] > 0.0 && sigma[1][k] > 0.0)
        {
          s1 = std::min(sigma[2][k], 2.0 * sigma[1][k]);
          s2 = std::min(2.0 * sigma[2][k], sigma[1][k]);
        }
        else if (sigma[2][k] < 0.0 && sigma[1][k] < 0.0)
        {
          s1 = std::max(sigma[2][k], 2.0 * sigma[1][k]);
          s2 = std::max(2.0 * sigma[2][k], sigma[1][k]);
        }
        if (s1 > 0.0 && s2 > 0.0)
          grad[k](0) = std::max(s1, s2);
        else if (s1 < 0.0 && s2 < 0.0)
          grad[k](0) = std::min(s1, s2);
      }
      break;
    default:
      break;
  }

  return grad;
}
