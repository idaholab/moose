/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputePolycrystalElasticityTensor.h"
#include "RotationTensor.h"

template<>
InputParameters validParams<ComputePolycrystalElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
  params.addClassDescription("Compute an evolving elasticity tensor coupled to a grain growth phase field model.");
  params.addRequiredParam<UserObjectName>("grain_tracker", "Name of GrainTracker user object that provides RankFourTensors");
  params.addParam<Real>("length_scale", 1.0e-9, "Lengthscale of the problem, in meters");
  params.addParam<Real>("pressure_scale", 1.0e6, "Pressure scale of the problem, in pa");
  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

ComputePolycrystalElasticityTensor::ComputePolycrystalElasticityTensor(const InputParameters & parameters) :
    ComputeElasticityTensorBase(parameters),
    _length_scale(getParam<Real>("length_scale")),
    _pressure_scale(getParam<Real>("pressure_scale")),
    _grain_tracker(getUserObject<GrainDataTracker<RankFourTensor>>("grain_tracker")),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _D_elastic_tensor(_op_num),
    _JtoeV(6.24150974e18)
{
  // Loop over variables (ops)
  for (unsigned int op = 0; op < _op_num; ++op)
  {
    // Initialize variables
    _vals[op] = &coupledValue("v", op);

    // declare elasticity tensor derivative properties
    _D_elastic_tensor[op] = &declarePropertyDerivative<RankFourTensor>(_elasticity_tensor_name, getVar("v", op)->name());
  }
}

void
ComputePolycrystalElasticityTensor::computeQpElasticityTensor()
{
  // Get list of active order parameters from grain tracker
  const std::vector<std::pair<unsigned int, unsigned int> > & active_ops = _grain_tracker.getElementalValues(_current_elem->id());
  unsigned int n_active_ops = active_ops.size();

  if (n_active_ops < 1 && _t_step > 0)
    mooseError("No active order parameters");

  // Calculate elasticity tensor
  _elasticity_tensor[_qp].zero();
  Real sum_h = 0.0;
  for (unsigned int op = 0; op < n_active_ops; ++op)
  {
    const unsigned int grain_index = active_ops[op].first;
    const unsigned int op_index = active_ops[op].second;

    // Interpolation factor for elasticity tensors
    Real h = (1.0 + std::sin(libMesh::pi * ((*_vals[op_index])[_qp] - 0.5))) / 2.0;

    // Sum all rotated elasticity tensors
    _elasticity_tensor[_qp] += _grain_tracker.getData(grain_index) * h;
    sum_h += h;
  }

  const Real tol = 1.0e-10;
  sum_h = std::max(sum_h, tol);
  _elasticity_tensor[_qp] /= sum_h;

  // Calculate elasticity tensor derivative: Cderiv = dhdopi/sum_h * (Cop - _Cijkl)
  for (unsigned int op = 0; op < _op_num; ++op)
    (*_D_elastic_tensor[op])[_qp].zero();

  for (unsigned int op = 0; op < n_active_ops; ++op)
  {
    const unsigned int grain_index = active_ops[op].first;
    const unsigned int op_index = active_ops[op].second;

    Real dhdopi = libMesh::pi * std::cos(libMesh::pi * ((*_vals[op_index])[_qp] - 0.5)) / 2.0;
    RankFourTensor & C_deriv = (*_D_elastic_tensor[op_index])[_qp];

    C_deriv = (_grain_tracker.getData(grain_index) - _elasticity_tensor[_qp]) * dhdopi / sum_h;

    // Convert from XPa to eV/(xm)^3, where X is pressure scale and x is length scale;
    C_deriv *= _JtoeV * (_length_scale * _length_scale * _length_scale) * _pressure_scale;
  }
}
