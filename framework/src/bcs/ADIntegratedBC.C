//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADIntegratedBC.h"

// MOOSE includes
#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "ADUtils.h"

#include "libmesh/quadrature.h"

template <typename T>
InputParameters
ADIntegratedBCTempl<T>::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  params += ADFunctorInterface::validParams();
  return params;
}

template <typename T>
ADIntegratedBCTempl<T>::ADIntegratedBCTempl(const InputParameters & parameters)
  : IntegratedBCBase(parameters),
    MooseVariableInterface<T>(this,
                              false,
                              "variable",
                              Moose::VarKindType::VAR_NONLINEAR,
                              std::is_same<T, Real>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                           : Moose::VarFieldType::VAR_FIELD_VECTOR),
    ADFunctorInterface(this),
    _var(*this->mooseVariable()),
    _normals(_assembly.adNormals()),
    _ad_q_points(_assembly.adQPointsFace()),
    _test(_var.phiFace()),
    _grad_test(_var.adGradPhiFace()),
    _u(_var.adSln()),
    _grad_u(_var.adGradSln()),
    _ad_JxW(_assembly.adJxWFace()),
    _ad_coord(_assembly.adCoordTransformation()),
    _phi(_assembly.phi(_var)),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh"))
{
  _subproblem.haveADObjects(true);

  addMooseVariableDependency(this->mooseVariable());

  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));
    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i = 0; i < _diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _diag_save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "diag_save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

template <typename T>
void
ADIntegratedBCTempl<T>::computeResidual()
{
  _residuals.resize(_test.size(), 0);
  for (auto & r : _residuals)
    r = 0;

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
        _residuals[_i] += raw_value(_ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual());
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
        _residuals[_i] += raw_value(_JxW[_qp] * _coord[_qp] * computeQpResidual());

  _assembly.processResiduals(_residuals, _var.dofIndices(), _vector_tags, _var.scalingFactor());

  if (_has_save_in)
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_residuals.data(), _save_in[i]->dofIndices());
}

template <typename T>
void
ADIntegratedBCTempl<T>::computeResidualsForJacobian()
{
  if (_residuals_and_jacobians.size() != _test.size())
    _residuals_and_jacobians.resize(_test.size(), 0);
  for (auto & r : _residuals_and_jacobians)
    r = 0;

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      _r = _ad_JxW[_qp];
      _r *= _ad_coord[_qp];
      for (_i = 0; _i < _test.size(); _i++)
        _residuals_and_jacobians[_i] += _r * computeQpResidual();
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
        _residuals_and_jacobians[_i] += _JxW[_qp] * _coord[_qp] * computeQpResidual();
}

template <typename T>
void
ADIntegratedBCTempl<T>::computeResidualAndJacobian()
{
  computeResidualsForJacobian();
  _assembly.processResidualsAndJacobian(_residuals_and_jacobians,
                                        _var.dofIndices(),
                                        _vector_tags,
                                        _matrix_tags,
                                        _var.scalingFactor());
}

template <typename T>
void
ADIntegratedBCTempl<T>::addJacobian(const MooseVariableFieldBase & jvariable)
{
  unsigned int jvar = jvariable.number();

  auto ad_offset = Moose::adOffset(jvar, _sys.getMaxVarNDofsPerElem(), Moose::ElementType::Element);

  prepareMatrixTag(_assembly, _var.number(), jvar);

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jvariable.phiSize(); _j++)
      _local_ke(_i, _j) += _residuals_and_jacobians[_i].derivatives()[ad_offset + _j];
  accumulateTaggedLocalMatrix();
}

template <typename T>
void
ADIntegratedBCTempl<T>::computeJacobian()
{
  const std::vector<std::pair<MooseVariableFieldBase *, MooseVariableFieldBase *>>
      var_var_coupling = {std::make_pair(&_var, &_var)};
  computeADJacobian(var_var_coupling);

  if (_has_diag_save_in && !_sys.computingScalingJacobian())
    mooseError("_local_ke not computed for global AD indexing. Save-in is deprecated anyway. Use "
               "the tagging system instead.");
}

template <typename T>
void
ADIntegratedBCTempl<T>::computeADJacobian(
    const std::vector<std::pair<MooseVariableFieldBase *, MooseVariableFieldBase *>> &
        coupling_entries)
{
  computeResidualsForJacobian();

  auto local_functor =
      [&](const std::vector<ADReal> &, const std::vector<dof_id_type> &, const std::set<TagID> &)
  {
    for (const auto & it : coupling_entries)
    {
      MooseVariableFEBase & ivariable = *(it.first);
      MooseVariableFEBase & jvariable = *(it.second);

      unsigned int ivar = ivariable.number();

      if (ivar != _var.number() || !jvariable.hasBlocks(_current_elem->subdomain_id()))
        continue;

      // Make sure to get the correct undisplaced/displaced variable
      addJacobian(getVariable(jvariable.number()));
    }
  };

  _assembly.processJacobian(_residuals_and_jacobians,
                            _var.dofIndices(),
                            _matrix_tags,
                            _var.scalingFactor(),
                            local_functor);
}

template <typename T>
void
ADIntegratedBCTempl<T>::computeOffDiagJacobian(const unsigned int jvar)
{
  // Only need to do this once because AD does all the derivatives at once
  if (jvar == _var.number())
    computeADJacobian(_assembly.couplingEntries());
}

template <typename T>
void
ADIntegratedBCTempl<T>::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
}

template class ADIntegratedBCTempl<Real>;
template class ADIntegratedBCTempl<RealVectorValue>;
