//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADStressDivergenceTensors.h"
#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ADStressDivergenceTensors);
registerMooseObject("TensorMechanicsApp", ADSymmetricStressDivergenceTensors);

template <typename R2>
InputParameters
ADStressDivergenceTensorsTempl<R2>::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Stress divergence kernel with automatic differentiation for the "
                             "Cartesian coordinate system");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addCoupledVar("out_of_plane_strain",
                       "The name of the out_of_plane_strain variable used in the "
                       "WeakPlaneStress kernel.");
  params.set<bool>("use_displaced_mesh") = false;
  params.addParam<bool>("volumetric_locking_correction",
                        false,
                        "Set to false to turn off volumetric locking correction");
  return params;
}

template <typename R2>
ADStressDivergenceTensorsTempl<R2>::ADStressDivergenceTensorsTempl(
    const InputParameters & parameters)
  : ADKernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getADMaterialProperty<R2>(_base_name + "stress")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _avg_grad_test(),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? &adCoupledValue("out_of_plane_strain")
                                                      : nullptr),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction"))
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    // the next line should be _disp_var[i] = coupled("displacements", i);
    // but the Coupleable:: is required to avoid triggering an internal Intel compiler bug
    _disp_var[i] = Coupleable::coupled("displacements", i);

  // Error if volumetric locking correction is turned on for 1D problems
  if (_ndisp == 1 && _volumetric_locking_correction)
    mooseError("Volumetric locking correction should be set to false for 1-D problems.");
}

template <typename R2>
void
ADStressDivergenceTensorsTempl<R2>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_XYZ)
    mooseError(
        "The coordinate system in the Problem block must be set to XYZ for cartesian geometries.");
}

template <typename R2>
ADReal
ADStressDivergenceTensorsTempl<R2>::computeQpResidual()
{
  ADReal residual = _stress[_qp].row(_component) * _grad_test[_i][_qp];

  // volumetric locking correction
  if (_volumetric_locking_correction)
    residual += (_avg_grad_test[_i] - _grad_test[_i][_qp](_component)) / 3.0 * _stress[_qp].trace();

  if (_ndisp != 3 && _out_of_plane_strain_coupled && _use_displaced_mesh)
  {
    const ADReal out_of_plane_thickness = std::exp((*_out_of_plane_strain)[_qp]);
    residual *= out_of_plane_thickness;
  }

  return residual;
}

template <typename R2>
void
ADStressDivergenceTensorsTempl<R2>::precalculateResidual()
{
  if (!_volumetric_locking_correction)
    return;

  ADReal ad_current_elem_volume = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    ad_current_elem_volume += _ad_JxW[qp] * _ad_coord[qp];

  // Calculate volume averaged value of shape function derivative
  _avg_grad_test.resize(_test.size());
  for (_i = 0; _i < _test.size(); ++_i)
  {
    _avg_grad_test[_i] = 0.0;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      _avg_grad_test[_i] += _grad_test[_i][_qp](_component) * _ad_JxW[_qp] * _ad_coord[_qp];

    _avg_grad_test[_i] /= ad_current_elem_volume;
  }
}

template class ADStressDivergenceTensorsTempl<RankTwoTensor>;
template class ADStressDivergenceTensorsTempl<SymmetricRankTwoTensor>;
