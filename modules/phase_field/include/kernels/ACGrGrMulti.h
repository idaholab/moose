//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ACGrGrBase.h"
#include "ADGrainGrowthBase.h"

/**
 * This kernel calculates the residual for grain growth for a multi-phase,
 * poly-crystal system. A list of material properties needs to be supplied for the gammas
 * (prefactors of the cross-terms between order parameters).
 */

template <bool is_ad>
using ACGrGrMultiBase = typename std::conditional<is_ad, ADGrainGrowthBase, ACGrGrBase>::type;

template <bool is_ad>
class ACGrGrMultiTempl : public ACGrGrMultiBase<is_ad>
{
public:
  static InputParameters validParams();

  ACGrGrMultiTempl(const InputParameters & parameters);

protected:
  /// Names of gammas for each order parameter
  std::vector<MaterialPropertyName> _gamma_names;
  unsigned int _num_j;

  /// Values of gammas for each order parameter
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _prop_gammas;

  using ACGrGrMultiBase<is_ad>::_op_num;
  using ACGrGrMultiBase<is_ad>::_qp;
  using ACGrGrMultiBase<is_ad>::_vals;
  using ACGrGrMultiBase<is_ad>::_u;
  using ACGrGrMultiBase<is_ad>::_mu;

  GenericReal<is_ad> computedF0du();
};

class ACGrGrMulti : public ACGrGrMultiTempl<false>
{
public:
  ACGrGrMulti(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type) override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const NonlinearVariableName _uname;
  const MaterialProperty<Real> & _dmudu;
  const std::vector<VariableName> _vname;
  std::vector<const MaterialProperty<Real> *> _dmudEtaj;
};

class ADACGrGrMulti : public ACGrGrMultiTempl<true>
{
public:
  using ACGrGrMultiTempl<true>::ACGrGrMultiTempl;

protected:
  virtual ADReal computeDFDOP() override;
};
