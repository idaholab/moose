/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeGrainCenterUserObject.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ComputeGrainCenterUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Userobject for calculating the grain volume and grain center");
  params.addRequiredCoupledVarWithAutoBuild("etas", "var_name_base", "op_num", "order parameters");
  return params;
}

ComputeGrainCenterUserObject::ComputeGrainCenterUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _ncrys(coupledComponents("etas")), // determine number of grains from the number of names passed
                                       // in.  Note this is the actual number -1
    _vals(_ncrys),                     // Size variable arrays
    _ncomp(4 * _ncrys),
    _grain_data(_ncomp),
    _grain_volumes(_ncrys),
    _grain_centers(_ncrys)
{
  for (unsigned int i = 0; i < _ncrys; ++i)
    _vals[i] = &coupledValue("etas", i);
}

void
ComputeGrainCenterUserObject::initialize()
{
  for (unsigned int i = 0; i < _ncomp; ++i)
    _grain_data[i] = 0;
}

void
ComputeGrainCenterUserObject::execute()
{
  for (unsigned int i = 0; i < _ncrys; ++i)
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      _grain_data[4 * i + 0] += _JxW[_qp] * _coord[_qp] * (*_vals[i])[_qp];
      _grain_data[4 * i + 1] += _JxW[_qp] * _coord[_qp] * _q_point[_qp](0) * (*_vals[i])[_qp];
      _grain_data[4 * i + 2] += _JxW[_qp] * _coord[_qp] * _q_point[_qp](1) * (*_vals[i])[_qp];
      _grain_data[4 * i + 3] += _JxW[_qp] * _coord[_qp] * _q_point[_qp](2) * (*_vals[i])[_qp];
    }
}

void
ComputeGrainCenterUserObject::finalize()
{
  gatherSum(_grain_data);

  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _grain_volumes[i] = _grain_data[4 * i + 0];
    _grain_centers[i](0) = _grain_data[4 * i + 1] / _grain_volumes[i];
    _grain_centers[i](1) = _grain_data[4 * i + 2] / _grain_volumes[i];
    _grain_centers[i](2) = _grain_data[4 * i + 3] / _grain_volumes[i];
  }
}

void
ComputeGrainCenterUserObject::threadJoin(const UserObject & y)
{
  const ComputeGrainCenterUserObject & pps = static_cast<const ComputeGrainCenterUserObject &>(y);
  for (unsigned int i = 0; i < _ncomp; ++i)
    _grain_data[i] += pps._grain_data[i];
}

const std::vector<Real> &
ComputeGrainCenterUserObject::getGrainVolumes() const
{
  return _grain_volumes;
}

const std::vector<Point> &
ComputeGrainCenterUserObject::getGrainCenters() const
{
  return _grain_centers;
}
