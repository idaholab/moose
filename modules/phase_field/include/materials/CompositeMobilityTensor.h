/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPOSITEMOBILITYTENSOR_H
#define COMPOSITEMOBILITYTENSOR_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class CompositeMobilityTensor;

template<>
InputParameters validParams<CompositeMobilityTensor>();

/**
 * CompositeMobilityTensor provides a simple RealTensorValue type
 * MaterialProperty that can be used as a mobility in a phase field simulation.
 */
class CompositeMobilityTensor : public DerivativeMaterialInterface<Material>
{
public:
  CompositeMobilityTensor(const InputParameters & parameters);

protected:
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
  MaterialProperty<RealTensorValue> & _M;
  std::vector<MaterialProperty<RealTensorValue> *> _dM;
  std::vector<std::vector<MaterialProperty<RealTensorValue> *> > _d2M;
  /// @}

  /// @{ component tensors and their derivatives w.r.t. the args
  std::vector<const MaterialProperty<RealTensorValue> *> _tensors;
  std::vector<std::vector<const MaterialProperty<RealTensorValue> *> > _dtensors;
  std::vector<std::vector<std::vector<const MaterialProperty<RealTensorValue> *> > > _d2tensors;
  /// @}

  /// @{ component weights and their derivatives w.r.t. the args
  std::vector<const MaterialProperty<Real> *> _weights;
  std::vector<std::vector<const MaterialProperty<Real> *> > _dweights;
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *> > > _d2weights;
  /// @}
};

#endif //COMPOSITEMOBILITYTENSOR_H
