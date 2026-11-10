//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelUniqueId.h"
#include "FEProblemBase.h"
#include "MaterialBase.h"
#include "ThreadedElementLoopBase.h"
#include "ConsoleUtils.h"
#include "SwapBackSentinel.h"

// Forward declarations
class SystemBase;

/**
 * This mutex is used by all derived classes of the ThreadedElementLoop. It
 * is necessary to protect the creation of the strings used in the propagation
 * of the error messages.  It's possible for a thread to have acquired the
 * commonly used mutex in the Threads namespace so this one is here to
 * avoid any deadlocking.
 */
static Threads::spin_mutex threaded_element_mutex;

/**
 * Base class for assembly-like calculations.
 */
template <typename RangeType>
class ThreadedElementLoop : public ThreadedElementLoopBase<RangeType>
{
public:
  ThreadedElementLoop(FEProblemBase & feproblem);

  ThreadedElementLoop(ThreadedElementLoop & x, Threads::split split);

  virtual ~ThreadedElementLoop();

  virtual void caughtMooseException(MooseException & e) override;

  virtual bool keepGoing() override { return !_fe_problem.hasException(); }

  virtual void preElement(const Elem * elem) override;

  virtual void preInternalSide(const Elem * elem, unsigned int side) override;

  virtual void preBoundary(const Elem * elem,
                           unsigned int side,
                           BoundaryID bnd_id,
                           const Elem * lower_d_elem = nullptr) override;

  virtual void neighborSubdomainChanged() override;

protected:
  void prepareElement(const Elem * elem);
  void clearVarsAndMaterials();
  /// Determine the face and boundary materials required by material property consumers.
  void getRequiredBoundaryMaterials(
      const std::vector<const MaterialPropertyInterface *> & material_consumers,
      BoundaryID bnd_id,
      SubdomainID subdomain_id,
      std::deque<MaterialBase *> & required_face_materials,
      std::deque<MaterialBase *> & required_boundary_materials) const;
  FEProblemBase & _fe_problem;

  /**
   * Routine to output the ordering of objects within a vector of pointers to these objects.
   * These objects must implement the name() routine, and it must return a string or compatible
   * type.
   *
   * @tparam T the object type
   * @param objs the vector with all the objects (should be pointers)
   * @param objects_type the name of the type of objects. Defaults to the CPP object name
   * @param print_header whether to print a header about the timing of execution and the type of
   * objects
   */
  template <typename T>
  void printExecutionOrdering(const std::vector<T *> & objs,
                              const bool print_header = true,
                              const std::string & line_prefix = "[DBG]") const;
  template <typename T>
  void printExecutionOrdering(const std::vector<std::shared_ptr<T>> & objs_ptrs,
                              const bool print_header = true,
                              const std::string & line_prefix = "[DBG]") const;
};

template <typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(FEProblemBase & fe_problem)
  : ThreadedElementLoopBase<RangeType>(fe_problem.mesh()), _fe_problem(fe_problem)
{
}

template <typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(ThreadedElementLoop & x,
                                                    Threads::split /*split*/)
  : ThreadedElementLoopBase<RangeType>(x), _fe_problem(x._fe_problem)
{
}

template <typename RangeType>
ThreadedElementLoop<RangeType>::~ThreadedElementLoop()
{
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::caughtMooseException(MooseException & e)
{
  Threads::spin_mutex::scoped_lock lock(threaded_element_mutex);

  std::string what(e.what());
  _fe_problem.setException(what);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::preElement(const Elem * el)
{
  _fe_problem.setCurrentSubdomainID(el, ThreadedElementLoopBase<RangeType>::_tid);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::preInternalSide(const Elem * el, unsigned int side)
{
  _fe_problem.setNeighborSubdomainID(el, side, ThreadedElementLoopBase<RangeType>::_tid);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::preBoundary(const Elem * /*elem*/,
                                            unsigned int /*side*/,
                                            BoundaryID bnd_id,
                                            const Elem * /*=nullptr*/)
{
  _fe_problem.setCurrentBoundaryID(bnd_id, ThreadedElementLoopBase<RangeType>::_tid);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::neighborSubdomainChanged()
{
  _fe_problem.neighborSubdomainSetup(ThreadedElementLoopBase<RangeType>::_neighbor_subdomain,
                                     ThreadedElementLoopBase<RangeType>::_tid);
}

template <typename RangeType>
template <typename T>
void
ThreadedElementLoop<RangeType>::printExecutionOrdering(const std::vector<T *> & objs,
                                                       const bool print_header,
                                                       const std::string & line_prefix) const
{
  if (!objs.size())
    return;

  auto & console = _fe_problem.console();
  const auto objects_type = MooseUtils::prettyCppType(objs[0]);
  std::vector<MooseObject *> moose_objs;
  for (auto obj_ptr : objs)
    moose_objs.push_back(dynamic_cast<MooseObject *>(obj_ptr));
  const auto names = ConsoleUtils::mooseObjectVectorToString(moose_objs);

  // Print string with a DBG prefix and with sufficient line breaks
  std::string message = print_header ? "Executing " + objects_type + " on " +
                                           _fe_problem.getCurrentExecuteOnFlag().name() + "\n"
                                     : "";
  message += (print_header ? "Order of execution:\n" : "") + names;
  console << ConsoleUtils::formatString(message, line_prefix) << std::endl;
}

template <typename RangeType>
template <typename T>
void
ThreadedElementLoop<RangeType>::printExecutionOrdering(
    const std::vector<std::shared_ptr<T>> & objs_ptrs,
    const bool print_header,
    const std::string & line_prefix) const
{
  std::vector<T *> regular_ptrs;
  for (auto shared_ptr : objs_ptrs)
    regular_ptrs.push_back(shared_ptr.get());
  printExecutionOrdering<T>(regular_ptrs, print_header, line_prefix);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::prepareElement(const Elem * const elem)
{
  _fe_problem.prepare(elem, this->_tid);
  _fe_problem.reinitElem(elem, this->_tid);
  _fe_problem.reinitMaterials(this->_subdomain, this->_tid);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::clearVarsAndMaterials()
{
  _fe_problem.clearActiveElementalMooseVariables(this->_tid);
  _fe_problem.clearActiveMaterialProperties(this->_tid);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::getRequiredBoundaryMaterials(
    const std::vector<const MaterialPropertyInterface *> & material_consumers,
    const BoundaryID bnd_id,
    const SubdomainID subdomain_id,
    std::deque<MaterialBase *> & required_face_materials,
    std::deque<MaterialBase *> & required_boundary_materials) const
{
  required_face_materials.clear();
  required_boundary_materials.clear();

  std::unordered_set<unsigned int> needed_face_props;
  for (const auto * const consumer : material_consumers)
  {
    const auto & dependencies = consumer->getMatPropDependencies();
    needed_face_props.insert(dependencies.begin(), dependencies.end());
  }

  const auto & materials = _fe_problem.getRegularMaterialsWarehouse();
  // Resolve boundary materials first. A boundary material can depend on a property supplied by a
  // face material, while a property supplied by the boundary-material chain does not also need a
  // face producer.
  if (!material_consumers.empty() && materials.hasActiveBoundaryObjects(bnd_id, this->_tid))
    required_boundary_materials = MaterialBase::buildRequiredMaterials(
        material_consumers, materials.getActiveBoundaryObjects(bnd_id, this->_tid), true);
  for (const auto * const material : required_boundary_materials)
  {
    const auto & dependencies = material->getMatPropDependencies();
    needed_face_props.insert(dependencies.begin(), dependencies.end());
  }
  // The boundary-material chain has already satisfied these properties. Removing them prevents a
  // face material that happens to declare the same property from being selected unnecessarily.
  for (const auto * const material : required_boundary_materials)
    for (const auto supplied_prop : material->getSuppliedPropIDs())
      needed_face_props.erase(supplied_prop);

  struct MaterialDependencyConsumer
  {
    const std::unordered_set<unsigned int> & dependencies;
    const std::unordered_set<unsigned int> & getMatPropDependencies() const { return dependencies; }
  };
  if (!needed_face_props.empty())
  {
    const auto & face_materials = materials[Moose::FACE_MATERIAL_DATA];
    if (face_materials.hasActiveBlockObjects(subdomain_id, this->_tid))
    {
      const MaterialDependencyConsumer face_consumer{needed_face_props};
      const std::vector<const MaterialDependencyConsumer *> face_consumers{&face_consumer};
      required_face_materials = MaterialBase::buildRequiredMaterials(
          face_consumers, face_materials.getActiveBlockObjects(subdomain_id, this->_tid), true);
    }
  }
}
