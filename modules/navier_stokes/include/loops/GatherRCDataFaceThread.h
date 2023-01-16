//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeFVFluxThread.h"
#include "INSFVMomentumResidualObject.h"
#include "INSFVAttributes.h"
#include "TheWarehouse.h"

/**
 * A class that gathers 'a' coefficient data from flux kernels, boundary conditions, and interface
 * kernels contributing to the Navier-Stokes momentum residuals. We loop over each active, local
 * face and call the gatherRCData method on each kernel with the current face as an argument
 */
template <typename RangeType>
class GatherRCDataFaceThread : public ThreadedFaceLoop<RangeType>
{
public:
  using Parent = ThreadedFaceLoop<RangeType>;
  using Parent::join;

  GatherRCDataFaceThread(FEProblemBase & fe_problem,
                         const unsigned int nl_sys_number,
                         const std::vector<unsigned int> & vars);

  // Splitting Constructor
  GatherRCDataFaceThread(GatherRCDataFaceThread & x, Threads::split split);

  void onFace(const FaceInfo & fi) override final;
  void onBoundary(const FaceInfo & fi, BoundaryID boundary) override final;
  void subdomainChanged() override final;
  void neighborSubdomainChanged() override final;

private:
  /**
   * Called at the end of either \p subdomainChanged or \p neighborSubdomainChanged, this method
   * computes the final \p _fv_flux_kernels set
   */
  void finalizeContainers();

  /**
   * This determines all the momentum residual objects for all the variables
   * @param ros The output of this method; all the momentum residual objects for all the variables
   * @param queries Candidate MooseObjects for momentum residual object consideration that have been
   * pre-filtered based on attributes such as thread ID, boundary ID, subdomain ID, etc.
   */
  template <typename... Attribs>
  void getVarROs(std::vector<INSFVMomentumResidualObject *> & ros,
                 TheWarehouse::QueryCache<Attribs...> & queries);

  /// The velocity variable numbers
  const std::vector<unsigned int> & _vars;

  /// The collection of flux kernels that contribute to the momentum equation residuals
  std::set<INSFVMomentumResidualObject *> _fv_flux_kernels;

  /// The subset of flux kernels that contribute to the momentum equation residual from the
  /// element side of the face
  std::set<INSFVMomentumResidualObject *> _elem_sub_fv_flux_kernels;

  /// The subset of flux kernels that contribute to the momentum equation residual from the
  /// neighbor side of the face
  std::set<INSFVMomentumResidualObject *> _neigh_sub_fv_flux_kernels;
};

template <typename RangeType>
GatherRCDataFaceThread<RangeType>::GatherRCDataFaceThread(FEProblemBase & fe_problem,
                                                          const unsigned int nl_sys_number,
                                                          const std::vector<unsigned int> & vars)
  : ThreadedFaceLoop<RangeType>(fe_problem, nl_sys_number, {}), _vars(vars)
{
}

template <typename RangeType>
GatherRCDataFaceThread<RangeType>::GatherRCDataFaceThread(GatherRCDataFaceThread & x,
                                                          Threads::split split)
  : ThreadedFaceLoop<RangeType>(x, split), _vars(x._vars)
{
}

template <typename RangeType>
template <typename... Attribs>
void
GatherRCDataFaceThread<RangeType>::getVarROs(std::vector<INSFVMomentumResidualObject *> & ros,
                                             TheWarehouse::QueryCache<Attribs...> & queries)
{
  for (const auto var_num : _vars)
  {
    // We don't want to do cascading var num attributes or else the second time around we won't get
    // any results out of the query (e.g. an object cannot have a variable that simultaneously has
    // both var number 0 and 1)
    auto copied_queries = queries;
    std::vector<INSFVMomentumResidualObject *> var_ros;
    copied_queries.template condition<AttribVar>(static_cast<int>(var_num)).queryInto(var_ros);
    for (auto * const var_ro : var_ros)
      ros.push_back(var_ro);
  }
}

template <typename RangeType>
void
GatherRCDataFaceThread<RangeType>::onFace(const FaceInfo & fi)
{
  for (auto * const k : _fv_flux_kernels)
    k->gatherRCData(fi);
}

template <typename RangeType>
void
GatherRCDataFaceThread<RangeType>::onBoundary(const FaceInfo & fi, BoundaryID bnd_id)
{
  {
    std::vector<INSFVMomentumResidualObject *> bcs;
    // cannot bind to lvalue reference otherwise the temporary is destroyed and we are left with a
    // dangling reference
    auto queries = this->_fe_problem.theWarehouse()
                       .query()
                       .template condition<AttribSystem>("FVFluxBC")
                       .template condition<AttribSysNum>(this->_nl_system_num)
                       .template condition<AttribThread>(this->_tid)
                       .template condition<AttribBoundaries>(bnd_id);
    getVarROs(bcs, queries);

    for (auto * const bc : bcs)
      bc->gatherRCData(fi);
  }

  {
    std::vector<INSFVMomentumResidualObject *> iks;
    // cannot bind to lvalue reference otherwise the temporary is destroyed and we are left with a
    // dangling reference
    auto queries = this->_fe_problem.theWarehouse()
                       .query()
                       .template condition<AttribSystem>("FVInterfaceKernel")
                       .template condition<AttribSysNum>(this->_nl_system_num)
                       .template condition<AttribThread>(this->_tid)
                       .template condition<AttribBoundaries>(bnd_id);
    getVarROs(iks, queries);

    for (auto * const ik : iks)
      ik->gatherRCData(fi);
  }
}

template <typename RangeType>
void
GatherRCDataFaceThread<RangeType>::subdomainChanged()
{
  ThreadedFaceLoop<RangeType>::subdomainChanged();

  // Clear kernels
  _fv_flux_kernels.clear();
  _elem_sub_fv_flux_kernels.clear();

  std::vector<INSFVMomentumResidualObject *> kernels;
  // cannot bind to lvalue reference otherwise the temporary is destroyed and we are left with a
  // dangling reference
  auto queries = this->_fe_problem.theWarehouse()
                     .query()
                     .template condition<AttribSysNum>(this->_nl_system_num)
                     .template condition<AttribSystem>("FVFluxKernel")
                     .template condition<AttribSubdomains>(this->_subdomain)
                     .template condition<AttribThread>(this->_tid);
  getVarROs(kernels, queries);

  _elem_sub_fv_flux_kernels =
      std::set<INSFVMomentumResidualObject *>(kernels.begin(), kernels.end());

  finalizeContainers();
}

template <typename RangeType>
void
GatherRCDataFaceThread<RangeType>::neighborSubdomainChanged()
{
  ThreadedFaceLoop<RangeType>::neighborSubdomainChanged();

  // Clear kernels
  _fv_flux_kernels.clear();
  _neigh_sub_fv_flux_kernels.clear();

  std::vector<INSFVMomentumResidualObject *> kernels;
  // cannot bind to lvalue reference otherwise the temporary is destroyed and we are left with a
  // dangling reference
  auto queries = this->_fe_problem.theWarehouse()
                     .query()
                     .template condition<AttribSysNum>(this->_nl_system_num)
                     .template condition<AttribSystem>("FVFluxKernel")
                     .template condition<AttribSubdomains>(this->_neighbor_subdomain)
                     .template condition<AttribThread>(this->_tid);
  getVarROs(kernels, queries);

  _neigh_sub_fv_flux_kernels =
      std::set<INSFVMomentumResidualObject *>(kernels.begin(), kernels.end());

  finalizeContainers();
}

template <typename RangeType>
void
GatherRCDataFaceThread<RangeType>::finalizeContainers()
{
  const bool same_kernels = _elem_sub_fv_flux_kernels == _neigh_sub_fv_flux_kernels;
  if (same_kernels)
    _fv_flux_kernels = _elem_sub_fv_flux_kernels;
  else
    std::set_union(_elem_sub_fv_flux_kernels.begin(),
                   _elem_sub_fv_flux_kernels.end(),
                   _neigh_sub_fv_flux_kernels.begin(),
                   _neigh_sub_fv_flux_kernels.end(),
                   std::inserter(_fv_flux_kernels, _fv_flux_kernels.begin()));
}
