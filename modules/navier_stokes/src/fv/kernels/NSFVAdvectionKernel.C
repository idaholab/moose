//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVAdvectionKernel.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

#include "MooseVariableFieldBase.h"
#include "SystemBase.h"
#include "ADReal.h"    // Moose::derivInsert
#include "MooseMesh.h" // FaceInfo methods
#include "FVDirichletBC.h"

#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/vector_value.h"

registerMooseObject("NavierStokesApp", NSFVAdvectionKernel);

InputParameters
NSFVAdvectionKernel::validParams()
{
  InputParameters params = FVMatAdvection::validParams();
  params += NSFVAdvectionBase::validParams();

  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;

  return params;
}

NSFVAdvectionKernel::NSFVAdvectionKernel(const InputParameters & params)
  : FVMatAdvection(params),
    NSFVAdvectionBase(params),
    _rho_elem(getADMaterialProperty<Real>("rho")),
    _rho_neighbor(getNeighborADMaterialProperty<Real>("rho")),
    _mu_elem(getADMaterialProperty<Real>("mu")),
    _mu_neighbor(getNeighborADMaterialProperty<Real>("mu"))
{
}

void
NSFVAdvectionKernel::interpolate(Moose::FV::InterpMethod m,
                                 ADRealVectorValue & v,
                                 const ADRealVectorValue & elem_v,
                                 const ADRealVectorValue & neighbor_v)
{
  Moose::FV::interpolate(
      Moose::FV::InterpMethod::Average, v, elem_v, neighbor_v, *_face_info, true);

  if (m != Moose::FV::InterpMethod::RhieChow)
    return;

  // Get pressure gradient
  const VectorValue<ADReal> & grad_p = _p_var->adGradSln(*_face_info);

  // Get uncorrected pressure gradient
  const VectorValue<ADReal> & unc_grad_p = _p_var->uncorrectedAdGradSln(*_face_info);

  auto tup = Moose::FV::determineElemOneAndTwo(*_face_info, *_p_var);
  const Elem * const elem_one = std::get<0>(tup);
  const Elem * const elem_two = std::get<1>(tup);
  const bool elem_is_elem_one = std::get<2>(tup);
  mooseAssert(elem_is_elem_one
                  ? elem_one == &_face_info->elem() && elem_two == _face_info->neighborPtr()
                  : elem_one == _face_info->neighborPtr() && elem_two == &_face_info->elem(),
              "The determineElemOneAndTwo utility determined the wrong value for elem_is_elem_one");

  const Point & elem_one_centroid =
      elem_is_elem_one ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const Point * const elem_two_centroid =
      elem_two ? (elem_is_elem_one ? &_face_info->neighborCentroid() : &_face_info->elemCentroid())
               : nullptr;
  Real elem_one_volume = elem_is_elem_one ? _face_info->elemVolume() : _face_info->neighborVolume();
  Real elem_two_volume =
      elem_two ? (elem_is_elem_one ? _face_info->neighborVolume() : _face_info->elemVolume()) : 0;

  const auto & elem_one_mu = elem_is_elem_one ? _mu_elem[_qp] : _mu_neighbor[_qp];
  const auto & elem_one_rho = elem_is_elem_one ? _rho_elem[_qp] : _rho_neighbor[_qp];

  // Now we need to perform the computations of D
  const ADReal & elem_one_a = rcCoeff(*elem_one, elem_one_mu, elem_one_rho);

  mooseAssert(elem_two ? _subproblem.getCoordSystem(elem_one->subdomain_id()) ==
                             _subproblem.getCoordSystem(elem_two->subdomain_id())
                       : true,
              "Coordinate systems must be the same between the two elements");

  Real coord;
  coordTransformFactor(_subproblem, elem_one->subdomain_id(), elem_one_centroid, coord);

  elem_one_volume *= coord;

  const auto elem_one_D = elem_one_volume / elem_one_a;
  ADReal face_D;

  if (elem_two && this->hasBlocks(elem_two->subdomain_id()))
  {
    const auto & elem_two_mu = elem_is_elem_one ? _mu_neighbor[_qp] : _mu_elem[_qp];
    const auto & elem_two_rho = elem_is_elem_one ? _rho_neighbor[_qp] : _rho_elem[_qp];

    const ADReal & elem_two_a = rcCoeff(*elem_two, elem_two_mu, elem_two_rho);

    coordTransformFactor(_subproblem, elem_two->subdomain_id(), *elem_two_centroid, coord);
    elem_two_volume *= coord;

    const auto elem_two_D = elem_two_volume / elem_two_a;

    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           face_D,
                           elem_one_D,
                           elem_two_D,
                           *_face_info,
                           elem_is_elem_one);
  }
  else
    face_D = elem_one_D;

  // perform the pressure correction
  v -= face_D * (grad_p - unc_grad_p);
}

ADReal
NSFVAdvectionKernel::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal u_interface;

  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  Moose::FV::interpolate(_advected_interp_method,
                         u_interface,
                         _adv_quant_elem[_qp],
                         _adv_quant_neighbor[_qp],
                         v,
                         *_face_info,
                         true);
  return _normal * v * u_interface;
}

#endif
