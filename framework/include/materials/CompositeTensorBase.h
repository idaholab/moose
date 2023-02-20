//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"

/**
 * CompositeTensorBase computes a simple T type MaterialProperty \f$ \bar T\f$
 * that is summed up from a list of other T objects (tensors) \f$ T_i \f$ with
 * associated weights \f$ w_i \f$.
 * \f$ \bar T = \sum_i T_i\cdot w_i \f$
 * Derivatives w.r.t. variables are computed accordingly.
 * This base class is used by the CompositeMobilityTensor and
 * CompositeElasticityTensor classes.
 *
 * \tparam T The type of the objects to sum together
 */
template <class T, class U>
class CompositeTensorBase : public DerivativeMaterialInterface<U>
{
public:
  static InputParameters validParams();

  CompositeTensorBase(const InputParameters & parameters);

protected:
  /**
   * Output material properties are initialized here so that derived classes can
   * modify the name.
   */
  void initializeDerivativeProperties(const std::string name);

  /**
   * Fill in the
   *  * main tensor property given by M
   *  * derivatives, set up by initializeDerivativeProperties
   *
   * The root_property is kept separate from the derivatives to allow the application
   * of this template to the Eigenstrain calculation, which contributes derivatives to a different
   * material property (and uses a derivative_prefactor of -1).
   */
  virtual void computeQpTensorProperties(MaterialProperty<T> & M, Real derivative_prefactor = 1.0);

  /// component tensor names
  std::vector<MaterialPropertyName> _tensor_names;
  /// component weight names
  std::vector<MaterialPropertyName> _weight_names;

  /// number of dependent variables
  unsigned int _num_args;
  /// number of compomemt tensors and weights
  unsigned int _num_comp;

  /// @{ Composed tensor and its derivatives
  std::vector<MaterialProperty<T> *> _dM;
  std::vector<std::vector<MaterialProperty<T> *>> _d2M;
  /// @}

  /// @{ component tensors and their derivatives w.r.t. the args
  std::vector<const MaterialProperty<T> *> _tensors;
  std::vector<std::vector<const MaterialProperty<T> *>> _dtensors;
  std::vector<std::vector<std::vector<const MaterialProperty<T> *>>> _d2tensors;
  /// @}

  /// @{ component weights and their derivatives w.r.t. the args
  std::vector<const MaterialProperty<Real> *> _weights;
  std::vector<std::vector<const MaterialProperty<Real> *>> _dweights;
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _d2weights;
  /// @}
};

template <class T, class U>
CompositeTensorBase<T, U>::CompositeTensorBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<U>(parameters),
    _tensor_names(this->template getParam<std::vector<MaterialPropertyName>>("tensors")),
    _weight_names(this->template getParam<std::vector<MaterialPropertyName>>("weights")),
    _num_args(this->DerivativeMaterialInterface<U>::isCoupled("args")
                  ? this->DerivativeMaterialInterface<U>::coupledComponents("args")
                  : this->DerivativeMaterialInterface<U>::coupledComponents("coupled_variables")),
    _num_comp(_tensor_names.size()),
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
}

template <class T, class U>
InputParameters
CompositeTensorBase<T, U>::validParams()
{
  InputParameters params = U::validParams();
  params.addRequiredParam<std::vector<MaterialPropertyName>>("tensors", "Component tensors");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("weights", "Component weights");
  params.addRequiredCoupledVar("args", "variable dependencies for the prefactor");
  params.deprecateCoupledVar("args", "coupled_variables", "02/07/2024");
  return params;
}

template <class T, class U>
void
CompositeTensorBase<T, U>::initializeDerivativeProperties(const std::string name)
{
  // setup output composite tensor and derivatives
  for (unsigned int j = 0; j < _num_args; ++j)
  {
    const VariableName & jname =
        this->DerivativeMaterialInterface<U>::getVar("coupled_variables", j)->name();
    _dM[j] = &this->template declarePropertyDerivative<T>(name, jname);
    _d2M[j].resize(j + 1);

    for (unsigned int k = 0; k <= j; ++k)
    {
      const VariableName & kname =
          this->DerivativeMaterialInterface<U>::getVar("coupled_variables", k)->name();

      _d2M[j][k] = &this->template declarePropertyDerivative<T>(name, jname, kname);
    }
  }

  // setup input components and its derivatives
  for (unsigned int i = 0; i < _num_comp; ++i)
  {
    _tensors[i] = &this->template getMaterialPropertyByName<T>(_tensor_names[i]);
    _weights[i] = &this->template getMaterialPropertyByName<Real>(_weight_names[i]);

    _dtensors[i].resize(_num_args);
    _dweights[i].resize(_num_args);
    _d2tensors[i].resize(_num_args);
    _d2weights[i].resize(_num_args);

    for (unsigned int j = 0; j < _num_args; ++j)
    {
      const VariableName & jname =
          this->DerivativeMaterialInterface<U>::getVar("coupled_variables", j)->name();

      _dtensors[i][j] =
          &this->template getMaterialPropertyDerivativeByName<T>(_tensor_names[i], jname);
      _dweights[i][j] =
          &this->template getMaterialPropertyDerivativeByName<Real>(_weight_names[i], jname);

      _d2tensors[i][j].resize(j + 1);
      _d2weights[i][j].resize(j + 1);

      for (unsigned int k = 0; k <= j; ++k)
      {
        const VariableName & kname =
            this->DerivativeMaterialInterface<U>::getVar("coupled_variables", k)->name();

        _d2tensors[i][j][k] =
            &this->template getMaterialPropertyDerivativeByName<T>(_tensor_names[i], jname, kname);
        _d2weights[i][j][k] = &this->template getMaterialPropertyDerivativeByName<Real>(
            _weight_names[i], jname, kname);
      }
    }
  }
}

template <class T, class U>
void
CompositeTensorBase<T, U>::computeQpTensorProperties(MaterialProperty<T> & M,
                                                     Real derivative_prefactor)
{
  const unsigned int qp = this->DerivativeMaterialInterface<U>::_qp;

  M[qp].zero();
  for (unsigned int i = 0; i < _num_comp; ++i)
  {
    M[qp] += (*_tensors[i])[qp] * (*_weights[i])[qp];

    for (unsigned int j = 0; j < _num_args; ++j)
    {
      if (i == 0)
        (*_dM[j])[qp].zero();

      (*_dM[j])[qp] += derivative_prefactor * ((*_tensors[i])[qp] * (*_dweights[i][j])[qp] +
                                               (*_dtensors[i][j])[qp] * (*_weights[i])[qp]);

      for (unsigned int k = 0; k <= j; ++k)
      {
        if (i == 0)
          (*_d2M[j][k])[qp].zero();

        (*_d2M[j][k])[qp] +=
            derivative_prefactor * (2.0 * (*_dtensors[i][j])[qp] * (*_dweights[i][j])[qp] +
                                    (*_tensors[i])[qp] * (*_d2weights[i][j][k])[qp] +
                                    (*_d2tensors[i][j][k])[qp] * (*_weights[i])[qp]);
      }
    }
  }
}
