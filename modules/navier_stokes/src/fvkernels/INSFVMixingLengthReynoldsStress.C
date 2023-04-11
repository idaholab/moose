//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMixingLengthReynoldsStress.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"
#include "SystemBase.h"

registerMooseObject("NavierStokesApp", INSFVMixingLengthReynoldsStress);

InputParameters
INSFVMixingLengthReynoldsStress::validParams()
{
  InputParameters params = INSFVFluxKernel::validParams();
  params.addClassDescription(
      "Computes the force due to the Reynolds stress term in the incompressible"
      " Reynolds-averaged Navier-Stokes equations.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>("mixing_length", "Turbulent eddy mixing length.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  // We assume the worst, e.g. we are doing Rhie-Chow. In that case we need three layers. An 'a'
  // coefficient evaluation at a face will necessitate evaluation of *this* object at every face of
  // the adjoining element, necessitating a face gradient evaluation at those faces, necessitating a
  // cell gradient evaluation in neighboring elements, necessitating cell value evaluations in
  // neighbors of those neighbor elements
  params.set<unsigned short>("ghost_layers") = 3;
  return params;
}

INSFVMixingLengthReynoldsStress::INSFVMixingLengthReynoldsStress(const InputParameters & params)
  : INSFVFluxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _axis_index(getParam<MooseEnum>("momentum_component")),
    _u(getFunctor<ADReal>("u")),
    _v(params.isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(params.isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mixing_len(getFunctor<ADReal>("mixing_length"))
{
  if (_dim >= 2 && !_v)
    mooseError(
        "In two or more dimensions, the v velocity must be supplied using the 'v' parameter");
  if (_dim >= 3 && !_w)
    mooseError("In threedimensions, the w velocity must be supplied using the 'w' parameter");
}

ADReal
INSFVMixingLengthReynoldsStress::computeStrongResidual()
{
  constexpr Real offset = 1e-15; // prevents explosion of sqrt(x) derivative to infinity

  const auto face = makeCDFace(*_face_info);
  const auto state = determineState();

  const auto grad_u = _u.gradient(face, state);
  // Compute the dot product of the strain rate tensor and the normal vector
  // aka (grad_v + grad_v^T) * n_hat
  ADReal norm_strain_rate = grad_u(_axis_index) * _normal(0);
  ADRealVectorValue grad_v;
  ADRealVectorValue grad_w;
  if (_dim >= 2)
  {
    grad_v = _v->gradient(face, state);
    norm_strain_rate += grad_v(_axis_index) * _normal(1);
    if (_dim >= 3)
    {
      grad_w = _w->gradient(face, state);
      norm_strain_rate += grad_w(_axis_index) * _normal(2);
    }
  }
  const ADRealVectorValue & var_grad = _index == 0 ? grad_u : (_index == 1 ? grad_v : grad_w);
  norm_strain_rate += var_grad * _normal;

  ADReal symmetric_strain_tensor_norm = 2.0 * Utility::pow<2>(grad_u(0));
  if (_dim >= 2)
  {
    symmetric_strain_tensor_norm +=
        2.0 * Utility::pow<2>(grad_v(1)) + Utility::pow<2>(grad_v(0) + grad_u(1));
    if (_dim >= 3)
      symmetric_strain_tensor_norm += 2.0 * Utility::pow<2>(grad_w(2)) +
                                      Utility::pow<2>(grad_u(2) + grad_w(0)) +
                                      Utility::pow<2>(grad_v(2) + grad_w(1));
  }

  symmetric_strain_tensor_norm = std::sqrt(symmetric_strain_tensor_norm + offset);

  // Interpolate the mixing length to the face
  const ADReal mixing_len = _mixing_len(face, state);

  // Compute the eddy diffusivity
  ADReal eddy_diff = symmetric_strain_tensor_norm * mixing_len * mixing_len;

  const ADReal rho = _rho(face, state);

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    const auto dof_number = _face_info->elem().dof_number(_sys.number(), _var.number(), 0);
    // norm_strain_rate is a linear combination of degrees of freedom so it's safe to straight-up
    // index into the derivatives vector at the dof we care about
    _ae = norm_strain_rate.derivatives()[dof_number];
    _ae *= -rho * eddy_diff;
  }
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    const auto dof_number = _face_info->neighbor().dof_number(_sys.number(), _var.number(), 0);
    _an = norm_strain_rate.derivatives()[dof_number];
    _an *= rho * eddy_diff;
  }

  // Return the turbulent stress contribution to the momentum equation
  return -1 * rho * eddy_diff * norm_strain_rate;
}

void
INSFVMixingLengthReynoldsStress::gatherRCData(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());

  processResidualAndJacobian(computeStrongResidual() * (fi.faceArea() * fi.faceCoord()));

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(&fi.elem(), _index, _ae * (fi.faceArea() * fi.faceCoord()));
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(fi.neighborPtr(), _index, _an * (fi.faceArea() * fi.faceCoord()));
}
