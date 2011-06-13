/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "AsmBlock.h"
#include "SystemBase.h"


AsmBlock::AsmBlock(SystemBase & sys, CouplingMatrix * & cm, THREAD_ID tid) :
    _sys(sys),
    _cm(cm),
    _dof_map(_sys.dofMap()),
    _tid(tid)
{
}

AsmBlock::~AsmBlock()
{
}

void
AsmBlock::init()
{
  unsigned int n_vars = _sys.nVariables();
  _sub_dof_indices.resize(n_vars);
  _sub_Re.resize(n_vars);

  _sub_Ke.resize(n_vars);
  for (unsigned int i = 0; i < n_vars; ++i)
    _sub_Ke[i].resize(n_vars);
}

void
AsmBlock::prepare(const Elem * elem)
{
  unsigned int n_vars = _sys.nVariables();
  for (unsigned int ivar = 0; ivar < n_vars; ivar++)
    _dof_map.dof_indices(elem, _sub_dof_indices[ivar], ivar);

  for (unsigned int ivar = 0; ivar < n_vars; ivar++)
  {
    for (unsigned int jvar = 0; jvar < n_vars; jvar++)
    {
      _sub_Ke[ivar][jvar].resize(_sub_dof_indices[ivar].size(), _sub_dof_indices[jvar].size());
      _sub_Ke[ivar][jvar].zero();
    }
    _sub_Re[ivar].resize(_sub_dof_indices[ivar].size());
    _sub_Re[ivar].zero();
  }
}

void
AsmBlock::copyShapes(unsigned int var)
{
  // FIXME: ugly
  MooseVariable * v = _sys._vars[_tid].all()[var];

  _phi = v->phi();
  _grad_phi = v->gradPhi();
  _second_phi = v->secondPhi();
}

void
AsmBlock::copyFaceShapes(unsigned int var)
{
  // FIXME: ugly
  MooseVariable * v = _sys._vars[_tid].all()[var];

  _phi_face = v->phiFace();
  _grad_phi_face = v->gradPhiFace();
  _second_phi_face = v->secondPhiFace();
}

void
AsmBlock::addResidual(NumericVector<Number> & residual)
{
  for (unsigned int var = 0; var < _sys.nVariables(); ++var)
  {
    if (_sub_dof_indices[var].size() > 0)
    {
      _dof_map.constrain_element_vector(_sub_Re[var], _sub_dof_indices[var], false);

      // FIXME: ugly
      Real scaling_factor = _sys._vars[_tid].all()[var]->scalingFactor();
      if (scaling_factor != 1.0)
      {
        DenseVector<Number> re(_sub_Re[var]);
        re.scale(scaling_factor);
        residual.add_vector(re, _sub_dof_indices[var]);
      }
      else
        residual.add_vector(_sub_Re[var], _sub_dof_indices[var]);
    }
  }
}

void
AsmBlock::addJacobian(SparseMatrix<Number> & jacobian)
{
  for (unsigned int ivar = 0; ivar < _sys.nVariables(); ++ivar)
  {
    for (unsigned int jvar = 0; jvar < _sys.nVariables(); ++jvar)
    {
      if ((*_cm)(ivar, jvar) != 0)
      {
        if ((_sub_dof_indices[ivar].size() > 0) && (_sub_dof_indices[jvar].size() > 0))
        {
          _dof_map.constrain_element_matrix(_sub_Ke[ivar][jvar], _sub_dof_indices[ivar], _sub_dof_indices[jvar], false);

          // FIXME: ugly
          Real scaling_factor = _sys._vars[_tid].all()[ivar]->scalingFactor();
          if (scaling_factor != 1.0)
          {
            DenseMatrix<Number> ke(_sub_Ke[ivar][jvar]);
            ke.scale(scaling_factor);
            jacobian.add_matrix(ke, _sub_dof_indices[ivar], _sub_dof_indices[jvar]);
          }
          else
            jacobian.add_matrix(_sub_Ke[ivar][jvar], _sub_dof_indices[ivar], _sub_dof_indices[jvar]);
        }
      }
    }
  }
}

