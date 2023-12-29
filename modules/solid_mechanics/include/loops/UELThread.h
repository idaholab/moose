//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedElementLoop.h"
#include "AbaqusUserElement.h"
#include "libmesh/elem_range.h"
#include "AuxiliarySystem.h"

class UELThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  UELThread(FEProblemBase & fe_problem, AbaqusUserElement & uel_uo);

  // Splitting Constructor
  UELThread(UELThread & x, Threads::split split);

  void join(const UELThread &) {}

protected:
  virtual void subdomainChanged() override final;
  virtual void onElement(const Elem * elem) override final;

private:
  /// Current nonlinear system
  NonlinearSystemBase & _sys;

  /// Auxiliary system object
  AuxiliarySystem * _aux_sys;

  /// dof indices of all coupled variables
  std::vector<dof_id_type> _var_dof_indices;
  std::vector<dof_id_type> _all_dof_indices;
  std::vector<Real> _all_dof_values;
  std::vector<Real> _all_dof_increments;
  std::vector<Real> _all_udot_dof_values;
  std::vector<Real> _all_udotdot_dof_values;

  // vectors for aux variables
  std::vector<dof_id_type> _aux_var_dof_indices;
  std::vector<dof_id_type> _all_aux_var_dof_indices;
  std::vector<Real> _all_aux_var_dof_increments;
  std::vector<Real> _all_aux_var_dof_values;
  std::vector<Real> _aux_var_values_to_uel;

  DenseVector<Real> _local_re;
  DenseMatrix<Real> _local_ke;

  const std::vector<const MooseVariableFieldBase *> & _variables;

  const std::vector<const MooseVariableFieldBase *> & _aux_variables;

  /// reference to the userobject (to access parameters)
  AbaqusUserElement & _uel_uo;

  /// have a reference to the UEL plugin here
  const AbaqusUserElement::uel_t & _uel;

  /// parameters for the UEL plugin
  std::array<int, 5> _lflags;
  std::vector<Real> _coords;

  /// state variable copy
  std::vector<Real> _statev_copy;
};
