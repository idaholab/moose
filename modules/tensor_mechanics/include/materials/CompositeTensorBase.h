/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPOSITETENSORBASE_H
#define COMPOSITETENSORBASE_H

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
template<typename T>
class CompositeTensorBase : public DerivativeMaterialInterface<Material>
{
public:
  CompositeTensorBase(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /**
   * Output material properties are initialized here so that derived classes can
   * modify _M_name and customize its corresponding input parameters.
   */
  void initializeProperties();

  virtual void computeQpProperties();

  /// Name of the mobility tensor material property
  MaterialPropertyName _M_name;
  /// component tensor names
  std::vector<MaterialPropertyName> _tensor_names;
  /// component weight names
  std::vector<MaterialPropertyName> _weight_names;

  /// number of dependent variables
  unsigned int _num_args;
  /// number of compomemt tensors and weights
  unsigned int _num_comp;

  /// @{ Composed tensor and its derivatives
  MaterialProperty<T> * _M;
  std::vector<MaterialProperty<T> *> _dM;
  std::vector<std::vector<MaterialProperty<T> *> > _d2M;
  /// @}

  /// @{ component tensors and their derivatives w.r.t. the args
  std::vector<const MaterialProperty<T> *> _tensors;
  std::vector<std::vector<const MaterialProperty<T> *> > _dtensors;
  std::vector<std::vector<std::vector<const MaterialProperty<T> *> > > _d2tensors;
  /// @}

  /// @{ component weights and their derivatives w.r.t. the args
  std::vector<const MaterialProperty<Real> *> _weights;
  std::vector<std::vector<const MaterialProperty<Real> *> > _dweights;
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *> > > _d2weights;
  /// @}
};


template<typename T>
CompositeTensorBase<T>::CompositeTensorBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _tensor_names(getParam<std::vector<MaterialPropertyName> >("tensors")),
    _weight_names(getParam<std::vector<MaterialPropertyName> >("weights")),
    _num_args(coupledComponents("args")),
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

template<typename T>
InputParameters
CompositeTensorBase<T>::validParams()
{
  InputParameters params = ::validParams<Material>();
  params.addRequiredParam<std::vector<MaterialPropertyName> >("tensors", "Component tensors");
  params.addRequiredParam<std::vector<MaterialPropertyName> >("weights", "Component weights");
  params.addRequiredCoupledVar("args", "variable dependencies for the prefactor");
  return params;
}

template<typename T>
void
CompositeTensorBase<T>::initializeProperties()
{
  _M = &declareProperty<T>(_M_name);

  // setup output composite tensor and derivatives
  for (unsigned int j = 0; j < _num_args; ++j)
  {
    const VariableName & jname = getVar("args", j)->name();
    _dM[j] = &declarePropertyDerivative<T>(_M_name, jname);
    _d2M[j].resize(j+1);

    for (unsigned int k = 0; k <= j; ++k)
    {
      const VariableName & kname = getVar("args", k)->name();
      _d2M[j][k] = &declarePropertyDerivative<T>(_M_name, jname, kname);
    }
  }

  // setup input components and its derivatives
  for (unsigned int i = 0; i < _num_comp; ++i)
  {
    _tensors[i] = &getMaterialPropertyByName<T>(_tensor_names[i]);
    _weights[i] = &getMaterialPropertyByName<Real>(_weight_names[i]);

    _dtensors[i].resize(_num_args);
    _dweights[i].resize(_num_args);
    _d2tensors[i].resize(_num_args);
    _d2weights[i].resize(_num_args);

    for (unsigned int j = 0; j < _num_args; ++j)
    {
      const VariableName & jname = getVar("args", j)->name();

      _dtensors[i][j] = &getMaterialPropertyDerivativeByName<T>(_tensor_names[i], jname);
      _dweights[i][j] = &getMaterialPropertyDerivativeByName<Real>(_weight_names[i], jname);

      _d2tensors[i][j].resize(j+1);
      _d2weights[i][j].resize(j+1);

      for (unsigned int k = 0; k <= j; ++k)
      {
        const VariableName & kname = getVar("args", k)->name();

        _d2tensors[i][j][k] = &getMaterialPropertyDerivativeByName<T>(_tensor_names[i], jname, kname);
        _d2weights[i][j][k] = &getMaterialPropertyDerivativeByName<Real>(_weight_names[i], jname, kname);
      }
    }
  }
}

template<typename T>
void
CompositeTensorBase<T>::computeQpProperties()
{
  (*_M)[_qp].zero();
  for (unsigned int i = 0; i < _num_comp; ++i)
  {
    (*_M)[_qp] += (*_tensors[i])[_qp] * (*_weights[i])[_qp];

    for (unsigned int j = 0; j < _num_args; ++j)
    {
      if (i == 0)
        (*_dM[j])[_qp].zero();

      (*_dM[j])[_qp] +=   (*_tensors[i])[_qp] * (*_dweights[i][j])[_qp]
                        + (*_dtensors[i][j])[_qp] * (*_weights[i])[_qp];

      for (unsigned int k = 0; k <= j; ++k)
      {
        if (i == 0)
          (*_d2M[j][k])[_qp].zero();

        (*_d2M[j][k])[_qp] +=   2.0 * (*_dtensors[i][j])[_qp] * (*_dweights[i][j])[_qp]
                              + (*_tensors[i])[_qp] * (*_d2weights[i][j][k])[_qp]
                              + (*_d2tensors[i][j][k])[_qp] * (*_weights[i])[_qp];
      }
    }
  }
}

#endif //COMPOSITETENSORBASE_H
