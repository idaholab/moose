//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDispatcher.h"
#include "KokkosVariableValue.h"
#include "KokkosMaterialPropertyValue.h"
#include "KokkosAssembly.h"
#include "KokkosSystem.h"

#include "AuxKernelBase.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for a user to derive their own Kokkos auxiliary kernels.
 *
 * The user should define computeValue() as inlined public method in their derived class (not
 * virtual override). The signature of computeQpResidual() expected to be defined in the derived
 * class is as follows:
 *
 * @param qp The local quadrature point index
 * @param datum The ResidualDatum object of the current thread
 * @returns The value at the quadrature point
 *
 * KOKKOS_FUNCTION Real computeValue(const unsigned int qp, ResidualDatum & datum) const;
 */
class AuxKernel : public ::AuxKernelBase,
                  public MeshHolder,
                  public AssemblyHolder,
                  public SystemHolder
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  AuxKernel(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  AuxKernel(const AuxKernel & object);

  // Unused for Kokkos auxiliary kernels because all elements are computed in parallel
  virtual void subdomainSetup() override final {}

  /**
   * Dispatch calculation
   */
  virtual void compute() override;

  /**
   * Get whether this auxiliary kernel is nodal
   * @returns Whether this auxiliary kernel is nodal
   */
  KOKKOS_FUNCTION bool isNodal() const { return _nodal; }

  /**
   * Kokkos function tags
   */
  ///@{
  struct ElementLoop
  {
  };
  struct NodeLoop
  {
  };
  ///@}

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ElementLoop, const ThreadID tid, const Derived & auxkernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(NodeLoop, const ThreadID tid, const Derived & auxkernel) const;
  ///@}

  /**
   * The parallel computation bodies that can be customized in the derived class by defining
   * them in the derived class with the same signature.
   * Make sure to define them as inlined public methods if to be defined in the derived class.
   */
  ///@{
  /**
   * Compute an element
   * @param kernel The auxiliary kernel object of the final derived type
   * @param datum The ResidualDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeElementInternal(const Derived & auxkernel,
                                              ResidualDatum & datum) const;
  /**
   * Compute a node
   * @param kernel The auxiliary kernel object of the final derived type
   * @param datum The ResidualDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeNodeInternal(const Derived & auxkernel, ResidualDatum & datum) const;
  ///@}

protected:
  /**
   * Retrieve the old value of the variable that this kernel operates on
   * @returns The old variable value object
   */
  VariableValue uOld() const;
  /**
   * Retrieve the older value of the variable that this kernel operates on
   * @returns The older variable value object
   */
  VariableValue uOlder() const;

  /**
   * Set element values to the auxiliary solution vector
   * @param values The array containing the solution values of the element
   * @param datum The ResidualDatum object of the current thread
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void setElementSolution(const Real * const values,
                                          const ResidualDatum & datum,
                                          const unsigned int comp = 0) const;
  /**
   * Set node value to the auxiliary solution vector
   * @param values The node solution value
   * @param datum The ResidualDatum object of the current thread
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void
  setNodeSolution(const Real value, const ResidualDatum & datum, const unsigned int comp = 0) const;

  /**
   * Flag whether this kernel is nodal
   */
  const bool _nodal;

  /**
   * Kokkos variable
   */
  Variable _kokkos_var;
  /**
   * Kokkos functor dispatchers
   */
  ///@{
  std::unique_ptr<DispatcherBase> _element_dispatcher;
  std::unique_ptr<DispatcherBase> _node_dispatcher;
  ///@}

  /**
   * Current test function
   */
  const VariableTestValue _test;
  /**
   * Current solution
   */
  const VariableValue _u;

  /**
   * TODO: Move to TransientInterface
   */
  ///@{
  /**
   * Time
   */
  Scalar<Real> _t;
  /**
   * Old time
   */
  Scalar<const Real> _t_old;
  /**
   * The number of the time step
   */
  Scalar<int> _t_step;
  /**
   * Time step size
   */
  Scalar<Real> _dt;
  /**
   * Size of the old time step
   */
  Scalar<Real> _dt_old;
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION void
AuxKernel::operator()(ElementLoop, const ThreadID tid, const Derived & auxkernel) const
{
  auto elem = kokkosBlockElementID(tid);

  ResidualDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var());

  auxkernel.computeElementInternal(auxkernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
AuxKernel::operator()(NodeLoop, const ThreadID tid, const Derived & auxkernel) const
{
  auto node = _bnd ? kokkosBoundaryNodeID(tid) : kokkosBlockNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  ResidualDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  auxkernel.computeNodeInternal(auxkernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
AuxKernel::computeElementInternal(const Derived & auxkernel, ResidualDatum & datum) const
{
  Real solution[MAX_CACHED_DOF];
  Real load[MAX_CACHED_DOF];
  Real mass[MAX_CACHED_DOF * MAX_CACHED_DOF];

  for (unsigned int i = 0; i < datum.n_dofs(); ++i)
  {
    solution[i] = 0;
    load[i] = 0;

    for (unsigned int j = 0; j < datum.n_dofs(); ++j)
      mass[j + datum.n_dofs() * i] = 0;
  }

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    auto value = auxkernel.computeValue(qp, datum);

    datum.reinit();

    for (unsigned int i = 0; i < datum.n_dofs(); ++i)
    {
      auto t = datum.JxW(qp) * _test(datum, i, qp);

      load[i] += t * value;

      for (unsigned int j = 0; j < datum.n_dofs(); ++j)
        mass[j + datum.n_dofs() * i] += t * _test(datum, j, qp);
    }
  }

  if (datum.n_dofs() == 1)
    // Mass matrix is simply the volume
    solution[0] = load[0] / mass[0];
  else
    Utils::choleskySolve(mass, solution, load, datum.n_dofs());

  setElementSolution(solution, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
AuxKernel::computeNodeInternal(const Derived & auxkernel, ResidualDatum & datum) const
{
  auto value = auxkernel.computeValue(0, datum);

  setNodeSolution(value, datum);
}

KOKKOS_FUNCTION inline void
AuxKernel::setElementSolution(const Real * const values,
                              const ResidualDatum & datum,
                              const unsigned int comp) const
{
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto var = _kokkos_var.var(comp);
  auto tag = _kokkos_var.tag();
  auto elem = datum.elem().id;

  for (unsigned int i = 0; i < datum.n_dofs(); ++i)
    sys.getVectorDofValue(sys.getElemLocalDofIndex(elem, i, var), tag) = values[i];
}

KOKKOS_FUNCTION inline void
AuxKernel::setNodeSolution(const Real value,
                           const ResidualDatum & datum,
                           const unsigned int comp) const
{
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto var = _kokkos_var.var(comp);
  auto tag = _kokkos_var.tag();
  auto node = datum.node();

  sys.getVectorDofValue(sys.getNodeLocalDofIndex(node, var), tag) = value;
}

} // namespace Kokkos
} // namespace Moose
