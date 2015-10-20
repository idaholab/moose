/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CompositeMobilityTensor.h"

template<>
InputParameters validParams<CompositeMobilityTensor>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Assemble a mobility tensor from multiple tensor contributions weighted by material properties");
  params.addRequiredParam<MaterialPropertyName>("M_name", "Name of the mobility tensor porperty to generate");
  params.addRequiredParam<std::vector<MaterialPropertyName> >("tensors", "Component tensors");
  params.addRequiredParam<std::vector<MaterialPropertyName> >("weights", "Component weights");
  params.addRequiredCoupledVar("args", "variable dependencies for the prefactor");
  return params;
}

CompositeMobilityTensor::CompositeMobilityTensor(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _M_name(getParam<MaterialPropertyName>("M_name")),
    _tensor_names(getParam<std::vector<MaterialPropertyName> >("tensors")),
    _weight_names(getParam<std::vector<MaterialPropertyName> >("weights")),
    _num_args(coupledComponents("args")),
    _num_comp(_tensor_names.size()),
    _M(declareProperty<RealTensorValue>(_M_name)),
    _dM(_num_args),
    _d2M(_num_args),
    _tensors(_num_comp),
    _dtensors(_num_comp),
    _d2tensors(_num_comp),
    _weights(_num_comp),
    _dweights(_num_comp),
    _d2weights(_num_comp)
{
  if (_num_comp != _weight_names.size())
    mooseError("The number of supplied 'tensors' and 'weights' must match.");

  // setup composite tensor and derivative
  for (unsigned int j = 0; j < _num_args; ++j)
  {
    const std::string & jname = getVar("args", j)->name();
    _dM[j] = &declarePropertyDerivative<RealTensorValue>(_M_name, jname);
    _d2M[j].resize(j+1);

    for (unsigned int k = 0; k <= j; ++k)
    {
      const std::string & kname = getVar("args", k)->name();
      _d2M[j][k] = &declarePropertyDerivative<RealTensorValue>(_M_name, jname, kname);
    }
  }

  // setup components and its derivatives
  for (unsigned int i = 0; i < _num_comp; ++i)
  {
    _tensors[i] = &getMaterialPropertyByName<RealTensorValue>(_tensor_names[i]);
    _weights[i] = &getMaterialPropertyByName<Real>(_weight_names[i]);

    _dtensors[i].resize(_num_args);
    _dweights[i].resize(_num_args);
    _d2tensors[i].resize(_num_args);
    _d2weights[i].resize(_num_args);

    for (unsigned int j = 0; j < _num_args; ++j)
    {
      const std::string & jname = getVar("args", j)->name();

      _dtensors[i][j] = &getMaterialPropertyDerivativeByName<RealTensorValue>(_tensor_names[i], jname);
      _dweights[i][j] = &getMaterialPropertyDerivativeByName<Real>(_weight_names[i], jname);

      _d2tensors[i][j].resize(j+1);
      _d2weights[i][j].resize(j+1);

      for (unsigned int k = 0; k <= j; ++k)
      {
        const std::string & kname = getVar("args", k)->name();

        _d2tensors[i][j][k] = &getMaterialPropertyDerivativeByName<RealTensorValue>(_tensor_names[i], jname, kname);
        _d2weights[i][j][k] = &getMaterialPropertyDerivativeByName<Real>(_weight_names[i], jname, kname);
      }
    }
  }
}

void
CompositeMobilityTensor::computeQpProperties()
{
  _M[_qp].zero();
  for (unsigned int i = 0; i < _num_comp; ++i)
  {
    _M[_qp] += (*_tensors[i])[_qp] * (*_weights[i])[_qp];
    for (unsigned int j = 0; j < _num_args; ++j)
    {
      (*_dM[j])[_qp] +=   (*_tensors[i])[_qp] * (*_dweights[i][j])[_qp]
                       + (*_dtensors[i][j])[_qp] * (*_weights[i])[_qp];

      for (unsigned int k = 0; k <= j; ++k)
        (*_d2M[j][k])[_qp] +=   2.0 * (*_dtensors[i][j])[_qp] * (*_dweights[i][j])[_qp]
                             + (*_tensors[i])[_qp] * (*_d2weights[i][j][k])[_qp]
                             + (*_d2tensors[i][j][k])[_qp] * (*_weights[i])[_qp];
    }
  }
}
