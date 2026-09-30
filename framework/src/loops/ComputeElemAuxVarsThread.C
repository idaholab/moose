//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeElemAuxVarsThread.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "AuxKernel.h"
#include "SwapBackSentinel.h"
#include "FEProblem.h"
#include "MaterialBase.h"
#include "ThreadedElementLoop.h"

#include "libmesh/threads.h"

template <typename AuxKernelType>
ComputeElemAuxVarsThread<AuxKernelType>::ComputeElemAuxVarsThread(FEProblemBase & problem,
                                                                  const TheWarehouse::Query & query,
                                                                  bool need_materials)
  : ThreadedElementLoop<ConstElemRange>(problem),
    _aux_sys(problem.getAuxiliarySystem()),
    _query(query),
    _query_subdomain(_query),
    _need_materials(need_materials)
{
}

// Splitting Constructor
template <typename AuxKernelType>
ComputeElemAuxVarsThread<AuxKernelType>::ComputeElemAuxVarsThread(ComputeElemAuxVarsThread & x,
                                                                  Threads::split /*split*/)
  : ThreadedElementLoop<ConstElemRange>(x._fe_problem),
    _aux_sys(x._aux_sys),
    _query(x._query),
    _query_subdomain(x._query_subdomain),
    _need_materials(x._need_materials)
{
}

template <typename AuxKernelType>
ComputeElemAuxVarsThread<AuxKernelType>::~ComputeElemAuxVarsThread()
{
}

template <typename AuxKernelType>
void
ComputeElemAuxVarsThread<AuxKernelType>::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  std::set<MooseVariableFEBase *> needed_moose_vars;
  std::unordered_set<unsigned int> needed_mat_props;
  std::set<TagID> needed_fe_var_matrix_tags;
  std::set<TagID> needed_fe_var_vector_tags;

  _fe_problem.getMaterialWarehouse().updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);

  std::vector<AuxKernelType *> kernels;
  _query_subdomain.queryInto(kernels, _tid, _subdomain);
  for (const auto & aux : kernels)
  {
    aux->subdomainSetup();
    const auto & mv_deps = aux->getMooseVariableDependencies();
    const auto & mp_deps = aux->getMatPropDependencies();
    needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    needed_mat_props.insert(mp_deps.begin(), mp_deps.end());

    auto & fe_var_coup_vtags = aux->getFEVariableCoupleableVectorTags();
    needed_fe_var_vector_tags.insert(fe_var_coup_vtags.begin(), fe_var_coup_vtags.end());

    auto & fe_var_coup_mtags = aux->getFEVariableCoupleableMatrixTags();
    needed_fe_var_matrix_tags.insert(fe_var_coup_mtags.begin(), fe_var_coup_mtags.end());
  }

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(needed_mat_props, _subdomain, _tid);
  _fe_problem.setActiveFEVariableCoupleableMatrixTags(needed_fe_var_matrix_tags, _tid);
  _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_fe_var_vector_tags, _tid);
}

template <typename AuxKernelType>
void
ComputeElemAuxVarsThread<AuxKernelType>::onElement(const Elem * elem)
{
  std::vector<AuxKernelType *> kernels;
  _query_subdomain.queryInto(kernels, _tid, _subdomain);
  if (kernels.size())
  {
    _fe_problem.prepare(elem, _tid);
    _fe_problem.reinitElem(elem, _tid);

    // Set up the sentinel so that, even if reinitMaterials() throws, we
    // still remember to swap back.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid, _need_materials);

    if (_need_materials)
      _fe_problem.reinitMaterials(elem->subdomain_id(), _tid);

    for (const auto & aux : kernels)
    {
      aux->compute();
      aux->variable().insert(_aux_sys.solution());

      // update the aux solution vector if writable coupled variables are used
      if (aux->hasWritableCoupledVariables())
      {
        for (auto * var : aux->getWritableCoupledVariables())
          var->insert(_aux_sys.solution());

        _fe_problem.reinitElem(elem, _tid);
      }
    }
  }
}

template <typename AuxKernelType>
void
ComputeElemAuxVarsThread<AuxKernelType>::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);

  _fe_problem.clearActiveFEVariableCoupleableVectorTags(_tid);
  _fe_problem.clearActiveFEVariableCoupleableMatrixTags(_tid);
}

template <typename AuxKernelType>
void
ComputeElemAuxVarsThread<AuxKernelType>::join(const ComputeElemAuxVarsThread & /*y*/)
{
}

template <typename AuxKernelType>
void
ComputeElemAuxVarsThread<AuxKernelType>::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;

  std::vector<AuxKernelType *> all_kernels;
  // clone the query until we have a const query, see #32362
  _query.clone().condition<AttribThread>(_tid).queryInto(all_kernels);
  if (all_kernels.empty())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Executing auxiliary kernels on elements on " << execute_on << std::endl;
}

template <typename AuxKernelType>
void
ComputeElemAuxVarsThread<AuxKernelType>::printBlockExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || _blocks_exec_printed.count(_subdomain))
    return;

  std::vector<AuxKernelType *> kernels;
  _query_subdomain.queryInto(kernels, _tid, _subdomain);
  if (kernels.empty())
    return;

  const auto & console = _fe_problem.console();
  console << "[DBG] Ordering of AuxKernels on block " << _subdomain << std::endl;
  printExecutionOrdering<AuxKernelType>(kernels, false);
  _blocks_exec_printed.insert(_subdomain);
}

template class ComputeElemAuxVarsThread<AuxKernel>;
template class ComputeElemAuxVarsThread<VectorAuxKernel>;
template class ComputeElemAuxVarsThread<ArrayAuxKernel>;
