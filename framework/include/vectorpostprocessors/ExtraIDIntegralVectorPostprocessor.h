//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVariableVectorPostprocessor.h"
#include "SpatialUserObjectFunctor.h"

/**
 * This ExtraIDIntegralVectorPostprocessor source code is to integrate variables based on parsed
 * extra IDs
 */
class ExtraIDIntegralVectorPostprocessor
  : public SpatialUserObjectFunctor<ElementVariableVectorPostprocessor>
{
public:
  static InputParameters validParams();
  ExtraIDIntegralVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;
  /// Return extra ID list
  const std::vector<VectorPostprocessorValue *> & getUniqueExtraIds() const { return _extra_ids; };
  /// Return Integral values
  const std::vector<VectorPostprocessorValue *> & getIntegrals() const { return _integrals; };

  using SpatialUserObjectFunctor<ElementVariableVectorPostprocessor>::evaluate;

  /// Get values for an element based on the element extra element id
  virtual Real evaluate(const ElemArg & elem, const Moose::StateArg & state) const override;
  /// Get values for an element on qps based on the element extra element id
  virtual Real evaluate(const ElemQpArg & qp, const Moose::StateArg & state) const override;

protected:
  /// Get the value for an element based on the element extra element id
  Real elementValue(const Elem * elem) const;
  /// whether or not to compute volume average
  const bool _average;
  /// Number of variables to be integrated
  const unsigned int _nvar;
  /// Number of material properties to be integrated
  const unsigned int _nprop;
  /// Name of material properties
  const std::vector<MaterialPropertyName> _prop_names;
  /// Extra IDs in use
  const std::vector<ExtraElementIDName> _extra_id;
  /// Number of extra IDs in use
  const unsigned int _n_extra_id;
  // Map of element ids to parsed vpp ids
  std::unordered_map<dof_id_type, dof_id_type> _unique_vpp_ids;
  /// Vectors holding extra IDs
  std::vector<VectorPostprocessorValue *> _extra_ids;
  /// Coupled MOOSE variables to be integrated
  std::vector<const MooseVariable *> _vars;
  /// Quadrature point values of coupled MOOSE variables
  std::vector<const VariableValue *> _var_values;
  /// Material properties to be integrated
  std::vector<const MaterialProperty<Real> *> _props;
  /// Vectors holding integrals over extra IDs
  std::vector<VectorPostprocessorValue *> _integrals;
  /// Vector holding the volume of extra IDs
  std::vector<Real> _volumes;
  /// The index to the values that is used in 'evaluate' function
  unsigned int _spatial_evaluation_index;
};
