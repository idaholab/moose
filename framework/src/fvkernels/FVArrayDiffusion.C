//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVArrayDiffusion.h"

registerMooseObject("MooseApp", FVArrayDiffusion);

InputParameters
FVArrayDiffusion::validParams()
{
  InputParameters params = FVArrayFluxKernel::validParams();
  params.addParam<MaterialPropertyName>("coeff",
                                        "The name of the diffusivity, "
                                        "can be scalar, vector, or matrix.");
  params.addClassDescription(
      "The array Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak "
      "form of $(\\nabla \\phi_i, \\nabla u_h)$.");
  return params;
}

FVArrayDiffusion::FVArrayDiffusion(const InputParameters & params)
  : FVArrayFluxKernel(params),
    _d_elem(hasADMaterialProperty<Real>("coeff") ? &getADMaterialProperty<Real>("coeff") : nullptr),
    _d_array_elem(hasADMaterialProperty<RealEigenVector>("coeff")
                      ? &getADMaterialProperty<RealEigenVector>("coeff")
                      : nullptr),
    _d_2d_array_elem(hasADMaterialProperty<RealEigenMatrix>("coeff")
                         ? &getADMaterialProperty<RealEigenMatrix>("coeff")
                         : nullptr),
    _d_neighbor(hasADMaterialProperty<Real>("coeff") ? &getNeighborADMaterialProperty<Real>("coeff")
                                                     : nullptr),
    _d_array_neighbor(hasADMaterialProperty<RealEigenVector>("coeff")
                          ? &getNeighborADMaterialProperty<RealEigenVector>("coeff")
                          : nullptr),
    _d_2d_array_neighbor(hasADMaterialProperty<RealEigenMatrix>("coeff")
                             ? &getNeighborADMaterialProperty<RealEigenMatrix>("coeff")
                             : nullptr)
{
}

ADRealEigenVector
FVArrayDiffusion::computeQpResidual()
{
  auto dudn = gradUDotNormal();

  if (_d_elem)
  {
    ADReal d;
    interpolate(
        Moose::FV::InterpMethod::Average, d, (*_d_elem)[_qp], (*_d_neighbor)[_qp], *_face_info);
    return -1 * d * dudn;
  }
  else if (_d_array_elem)
  {
    mooseAssert((*_d_array_elem)[_qp].size() == _var.count(),
                "coeff size is inconsistent with the number of components of array "
                "variable");
    ADRealEigenVector d;
    interpolate(Moose::FV::InterpMethod::Average,
                d,
                (*_d_array_elem)[_qp],
                (*_d_array_neighbor)[_qp],
                *_face_info);
    return -1 * d.cwiseProduct(dudn);
  }
  else
  {
    mooseAssert((*_d_2d_array_elem)[_qp].cols() == _var.count(),
                "coeff size is inconsistent with the number of components of array "
                "variable");
    mooseAssert((*_d_2d_array_elem)[_qp].rows() == _var.count(),
                "coeff size is inconsistent with the number of components of array "
                "variable");
    ADRealEigenMatrix d;
    interpolate(Moose::FV::InterpMethod::Average,
                d,
                (*_d_2d_array_elem)[_qp],
                (*_d_2d_array_neighbor)[_qp],
                *_face_info);
    return -1 * d * dudn;
  }
}
