//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseEigenSystem.h"

#include "MaterialData.h"
#include "Factory.h"
#include "EigenKernel.h"

MooseEigenSystem::MooseEigenSystem(FEProblemBase & fe_problem, const std::string & name)
  : NonlinearSystem(fe_problem, name),
    _all_eigen_vars(false),
    _active_on_old(false),
    _eigen_kernel_counter(0)
{
}

MooseEigenSystem::~MooseEigenSystem() {}

void
MooseEigenSystem::addKernel(const std::string & kernel_name,
                            const std::string & name,
                            InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // In the case of EigenKernels, we might need to add two to the system
    if (parameters.have_parameter<bool>("eigen"))
    {
      {
        // EigenKernel
        parameters.set<bool>("implicit") = true;
        std::shared_ptr<KernelBase> ekernel =
            _factory.create<KernelBase>(kernel_name, name, parameters, tid);
        if (parameters.get<bool>("eigen"))
          markEigenVariable(parameters.get<NonlinearVariableName>("variable"));
        _kernels.addObject(ekernel, tid);
      }
      if (parameters.get<bool>("eigen"))
      {
        // EigenKernel_old
        parameters.set<bool>("implicit") = false;
        std::string old_name(name + "_old");

        std::shared_ptr<KernelBase> ekernel =
            _factory.create<KernelBase>(kernel_name, old_name, parameters, tid);
        _eigen_var_names.insert(parameters.get<NonlinearVariableName>("variable"));
        _kernels.addObject(ekernel, tid);
        ++_eigen_kernel_counter;
      }
    }
    else // Standard nonlinear system kernel
    {
      // Create the kernel object via the factory
      std::shared_ptr<KernelBase> kernel =
          _factory.create<KernelBase>(kernel_name, name, parameters, tid);
      _kernels.addObject(kernel, tid);
    }
  }

  if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
MooseEigenSystem::markEigenVariable(const VariableName & var_name)
{
  _eigen_var_names.insert(var_name);
}

void
MooseEigenSystem::scaleSystemSolution(SYSTEMTAG tag, Real scaling_factor)
{
  if (tag == ALL)
  {
    solution().scale(scaling_factor);
  }
  else if (tag == EIGEN)
  {
    if (_all_eigen_vars)
    {
      solution().scale(scaling_factor);
    }
    else
    {
      for (const auto & dof : _eigen_var_indices)
        solution().set(dof, solution()(dof) * scaling_factor);
    }
  }
  solution().close();
  update();
}

void
MooseEigenSystem::combineSystemSolution(SYSTEMTAG tag, const std::vector<Real> & coefficients)
{
  mooseAssert(coefficients.size() > 0 && coefficients.size() <= 3, "Size error on coefficients");
  if (tag == ALL)
  {
    solution().scale(coefficients[0]);
    if (coefficients.size() > 1)
      solution().add(coefficients[1], solutionOld());
    if (coefficients.size() > 2)
      solution().add(coefficients[2], solutionOlder());
  }
  else if (tag == EIGEN)
  {
    if (_all_eigen_vars)
    {
      solution().scale(coefficients[0]);
      if (coefficients.size() > 1)
        solution().add(coefficients[1], solutionOld());
      if (coefficients.size() > 2)
        solution().add(coefficients[2], solutionOlder());
    }
    else
    {
      if (coefficients.size() > 2)
      {
        for (const auto & dof : _eigen_var_indices)
        {
          Real t = solution()(dof) * coefficients[0];
          t += solutionOld()(dof) * coefficients[1];
          t += solutionOlder()(dof) * coefficients[2];
          solution().set(dof, t);
        }
      }
      else if (coefficients.size() > 1)
      {
        for (const auto & dof : _eigen_var_indices)
        {
          Real t = solution()(dof) * coefficients[0];
          t += solutionOld()(dof) * coefficients[1];
          solution().set(dof, t);
        }
      }
      else
      {
        for (const auto & dof : _eigen_var_indices)
        {
          Real t = solution()(dof) * coefficients[0];
          solution().set(dof, t);
        }
      }
    }
  }
  solution().close();
  update();
}

void
MooseEigenSystem::initSystemSolution(SYSTEMTAG tag, Real v)
{
  if (tag == ALL)
  {
    solution() = v;
  }
  else if (tag == EIGEN)
  {
    if (_all_eigen_vars)
    {
      solution() = v;
    }
    else
    {
      for (const auto & dof : _eigen_var_indices)
        solution().set(dof, v);
    }
  }
  solution().close();
  update();
}

void
MooseEigenSystem::initSystemSolutionOld(SYSTEMTAG tag, Real v)
{
  if (tag == ALL)
  {
    solutionOld() = v;
  }
  else if (tag == EIGEN)
  {
    if (_all_eigen_vars)
    {
      solutionOld() = v;
    }
    else
    {
      for (const auto & dof : _eigen_var_indices)
        solutionOld().set(dof, v);
    }
  }
  solutionOld().close();
  update();
}

void
MooseEigenSystem::eigenKernelOnOld()
{
  _active_on_old = true;
  _fe_problem.updateActiveObjects(); // update warehouse active objects
}

void
MooseEigenSystem::eigenKernelOnCurrent()
{
  _active_on_old = false;
  _fe_problem.updateActiveObjects(); // update warehouse active objects
}

bool
MooseEigenSystem::activeOnOld()
{
  return _active_on_old;
}

void
MooseEigenSystem::buildSystemDoFIndices(SYSTEMTAG tag)
{
  if (tag == ALL)
  {
  }
  else if (tag == EIGEN)
  {
    // build DoF indices for the eigen system
    _eigen_var_indices.clear();
    _all_eigen_vars = getEigenVariableNames().size() == getVariableNames().size();
    if (!_all_eigen_vars)
    {
      for (std::set<VariableName>::const_iterator it = getEigenVariableNames().begin();
           it != getEigenVariableNames().end();
           it++)
      {
        unsigned int i = sys().variable_number(*it);
        std::set<dof_id_type> var_indices;
        sys().local_dof_indices(i, var_indices);
        _eigen_var_indices.insert(var_indices.begin(), var_indices.end());
      }
    }
  }
}

bool
MooseEigenSystem::containsEigenKernel() const
{
  return _eigen_kernel_counter > 0;
}
