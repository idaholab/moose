/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EigenstrainHelper.h"

#include "Assembly.h"
#include "MooseMesh.h"
#include "SubProblem.h"

// libmesh includes
#include "libmesh/quadrature.h"

EigenstrainHelper::EigenstrainHelper(const MooseObject * moose_object, bool bnd)
  : _eh_params(moose_object->parameters()),
    _eh_subproblem(*_eh_params.get<SubProblem *>("_subproblem")),
    _eh_tid(_eh_params.get<THREAD_ID>("_tid")),
    _eh_assembly(_eh_subproblem.assembly(_eh_tid)),
    _eh_qrule(bnd ? _eh_assembly.qRuleFace() : _eh_assembly.qRule()),
    _eh_JxW(bnd ? _eh_assembly.JxWFace() : _eh_assembly.JxW()),
    _eh_coord(_eh_assembly.coordTransformation()),
    _eh_q_point(bnd ? _eh_assembly.qPointsFace() : _eh_assembly.qPoints()),
    _eh_ncols(1 + _eh_subproblem.mesh().dimension()),
    _eh_second_order(_eh_subproblem.mesh().hasSecondOrderElements()),
    _eh_eigsum(),
    _eh_A(),
    _eh_b(6),
    _eh_AT(),
    _eh_ATb(_eh_ncols),
    _eh_x(6, DenseVector<Real>(_eh_ncols)),
    _eh_vals(6),
    _eh_adjusted_eigenstrain()
{
}

void
EigenstrainHelper::applyIncrementalEigenstrain(
    const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains,
    const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains_old,
    MaterialProperty<RankTwoTensor> & strain)
{
  if (eigenstrains.size() == 0)
    return;
  sumEigenstrainIncremental(eigenstrains, eigenstrains_old);
  applyEigenstrain(strain);
}

void
EigenstrainHelper::applyTotalEigenstrain(
    const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains,
    MaterialProperty<RankTwoTensor> & strain)
{
  if (eigenstrains.size() == 0)
    return;
  sumEigenstrainTotal(eigenstrains);
  applyEigenstrain(strain);
}

void
EigenstrainHelper::applyEigenstrain(MaterialProperty<RankTwoTensor> & strain)
{
  prepareEigenstrain();

  for (unsigned qp = 0; qp < _eh_qrule->n_points(); ++qp)
  {
    RankTwoTensor fillme;
    if (_eh_second_order)
    {
      for (unsigned i = 0; i < 6; ++i)
      {
        _eh_vals[i] = _eh_x[i](0);
        for (unsigned j = 1; j < _eh_ncols; ++j)
          _eh_vals[i] += _eh_x[i](j) * _eh_q_point[qp](j - 1);
      }
      _eh_adjusted_eigenstrain.fillFromInputVector(_eh_vals);
    }
    strain[qp] -= _eh_adjusted_eigenstrain;
  }
}

void
EigenstrainHelper::sumEigenstrainTotal(
    const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains)
{
  _eh_eigsum.resize(_eh_qrule->n_points());
  for (unsigned qp = 0; qp < _eh_qrule->n_points(); ++qp)
  {
    _eh_eigsum[qp].zero();
    for (auto es : eigenstrains)
      _eh_eigsum[qp] += (*es)[qp];
  }
}

void
EigenstrainHelper::sumEigenstrainIncremental(
    const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains,
    const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains_old)
{
  _eh_eigsum.resize(_eh_qrule->n_points());
  for (unsigned qp = 0; qp < _eh_qrule->n_points(); ++qp)
  {
    _eh_eigsum[qp].zero();
    for (unsigned i = 0; i < eigenstrains.size(); ++i)
    {
      _eh_eigsum[qp] += (*eigenstrains[i])[qp];
      _eh_eigsum[qp] -= (*eigenstrains_old[i])[qp];
    }
  }
}

void
EigenstrainHelper::prepareEigenstrain()
{
  // The eigenstrains can either be constant in an element or linear in x, y, z
  // If constant, do volume averaging.
  if (!_eh_second_order)
  {
    // Volume average
    _eh_adjusted_eigenstrain.zero();
    Real vol = 0.0;
    for (unsigned qp = 0; qp < _eh_qrule->n_points(); ++qp)
    {
      _eh_adjusted_eigenstrain += _eh_eigsum[qp] * _eh_JxW[qp] * _eh_coord[qp];
      vol += _eh_JxW[qp] * _eh_coord[qp];
    }
    _eh_adjusted_eigenstrain /= vol;
  }
  else
  {
    // 1 x y z

    // Six sets (one for each unique component of eigen) of qp data
    _eh_A.resize(_eh_qrule->n_points(), _eh_ncols);
    for (auto && b : _eh_b)
      b.resize(_eh_qrule->n_points());

    for (unsigned qp = 0; qp < _eh_qrule->n_points(); ++qp)
    {
      _eh_A(qp, 0) = 1.0;
      for (unsigned j = 1; j < _eh_ncols; ++j)
        _eh_A(qp, j) = _eh_q_point[qp](j - 1);

      _eh_b[0](qp) = _eh_eigsum[qp](0, 0);
      _eh_b[1](qp) = _eh_eigsum[qp](1, 1);
      _eh_b[2](qp) = _eh_eigsum[qp](2, 2);
      _eh_b[3](qp) = _eh_eigsum[qp](1, 2);
      _eh_b[4](qp) = _eh_eigsum[qp](0, 2);
      _eh_b[5](qp) = _eh_eigsum[qp](0, 1);
    }

    _eh_A.get_transpose(_eh_AT);
    _eh_A.left_multiply(_eh_AT);
    for (unsigned i = 0; i < 6; ++i)
    {
      _eh_AT.vector_mult(_eh_ATb, _eh_b[i]);

      _eh_A.cholesky_solve(_eh_ATb, _eh_x[i]);
    }
  }
}
