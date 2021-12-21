//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVariableVectorPostprocessor.h"

/**
 * This ExtraIDIntegralVectorPostprocessor source code is to integrate variables based on parsed
 * extra IDs
 */
class ExtraIDIntegralVectorPostprocessor : public ElementVariableVectorPostprocessor
{
public:
  static InputParameters validParams();
  ExtraIDIntegralVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;
  /// Return extra ID list
  const std::vector<VectorPostprocessorValue *> & getUniqueExtraIds() const
  {
    return _var_extra_ids;
  };
  /// Return Integral values
  const std::vector<VectorPostprocessorValue *> & getIntegrals() const { return _var_integrals; };

protected:
  /// Number of variables to be integrated
  const unsigned int _nvar;
  /// Extra IDs in use
  const std::vector<ExtraElementIDName> _extra_id;
  /// Number of extra IDs in use
  const unsigned int _n_extra_id;
  // Map of element ids to parsed vpp ids
  std::map<dof_id_type, dof_id_type> _unique_vpp_ids;
  /// Vectors holding extra IDs
  std::vector<VectorPostprocessorValue *> _var_extra_ids;
  /// Coupled MOOSE variables to be integrated
  std::vector<const MooseVariable *> _vars;
  /// Quadrature point values of coupled MOOSE variables
  std::vector<const VariableValue *> _var_values;
  /// Vectors holding variable integrals over extra IDs
  std::vector<VectorPostprocessorValue *> _var_integrals;
};
