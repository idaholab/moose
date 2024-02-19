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
 * Several kernels use a material property called mu. If mu is not a constant,
 * then this kernel will calculate the bulk AC term where mu is the derivative term.
 * It currently only takes a single value for gamma.
 **/
template <bool is_ad>
using ACBarrierFunctionBase = typename std::conditional<is_ad, ADGrainGrowthBase, ACGrGrBase>::type;

template <bool is_ad>
class ACBarrierFunctionTempl : public ACBarrierFunctionBase<is_ad>
{
public:
  static InputParameters validParams();

  ACBarrierFunctionTempl(const InputParameters & parameters);

protected:
  const NonlinearVariableName _uname;
  const MaterialPropertyName _gamma_name;
  const GenericMaterialProperty<Real, is_ad> & _gamma;
  const GenericMaterialProperty<Real, is_ad> & _dmudvar;

  using ACBarrierFunctionBase<is_ad>::_op_num;
  using ACBarrierFunctionBase<is_ad>::_qp;
  using ACBarrierFunctionBase<is_ad>::_vals;
  using ACBarrierFunctionBase<is_ad>::_u;
};

class ACBarrierFunction : public ACBarrierFunctionTempl<false>
{
public:
  ACBarrierFunction(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _n_eta;
  const MaterialProperty<Real> & _d2mudvar2;

  const std::vector<VariableName> _vname;
  std::vector<const MaterialProperty<Real> *> _d2mudvardeta;
  const JvarMap & _vmap;

private:
  Real calculateF0(); /// calculates the free energy function
};

class ADACBarrierFunction : public ACBarrierFunctionTempl<true>
{
public:
  using ACBarrierFunctionTempl<true>::ACBarrierFunctionTempl;

protected:
  virtual ADReal computeDFDOP();
};
