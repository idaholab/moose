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
#include "KokkosJaggedArray.h"
#include "KokkosVariableValue.h"
#include "KokkosMaterialPropertyValue.h"
#include "KokkosAssembly.h"
#include "KokkosFESystem.h"

#include "AuxKernelBase.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos auxiliary kernels.
 *
 * The user should define computeValue() as inlined public method in their derived class (not
 * virtual override). The signature of computeValue() expected to be defined in the derived
 * class is as follows:
 *
 * @tparam Derived The object type
 * @param qp The local quadrature point index
 * @param datum The AssemblyDatum object of the current thread
 * @returns The value at the quadrature point
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;
 */
class AuxKernel : public ::AuxKernelBase,
                  public MeshHolder,
                  public AssemblyHolder,
                  public FESystemHolder
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

  virtual void initialSetup() override;
  virtual void meshChanged() override;
  virtual void meshDisplaced() override;

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
  struct MassMatrixLoop
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
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(MassMatrixLoop, const ThreadID tid, const Derived & auxkernel) const;
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
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeElementInternal(const Derived & auxkernel,
                                              AssemblyDatum & datum) const;
  /**
   * Compute a node
   * @param kernel The auxiliary kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeNodeInternal(const Derived & auxkernel, AssemblyDatum & datum) const;
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
   * Compute the Cholesky factors of the element mass matrices and allocate the right-hand side and
   * solution storage used by computeElementInternal().
   * Called from initialSetup(), meshChanged(), and meshDisplaced(). A derived class whose
   * computeElementInternal() does not use them can override this to skip the computation.
   * @param rebuild Whether to rebuild the storage, which is needed when the elements or their
   * numbers of DOFs may have changed. Mesh displacement only moves nodes, so it keeps the storage
   * and only recomputes the factors.
   */
  virtual void setupMassFactors(const bool rebuild);

  /**
   * Set element values to the auxiliary solution vector
   * @param values The array containing the solution values of the element
   * @param datum The AssemblyDatum object of the current thread
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void setElementSolution(const Real * const values,
                                          const AssemblyDatum & datum,
                                          const unsigned int comp = 0) const;
  /**
   * Set node value to the auxiliary solution vector
   * @param values The node solution value
   * @param datum The AssemblyDatum object of the current thread
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void
  setNodeSolution(const Real value, const AssemblyDatum & datum, const unsigned int comp = 0) const;

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

private:
  /**
   * Override of the MaterialPropertyInterface function to throw on material property request for
   * nodal kernels
   */
  void getKokkosMaterialPropertyHook(const std::string & prop_name_in,
                                     const unsigned int state) override final;

  /**
   * Kokkos FE type ID of the variable
   */
  unsigned int _fe_type = libMesh::invalid_uint;
  /**
   * Cholesky factors of the element mass matrices of the L2 projection, indexed by the contiguous
   * element ID, each packed as the row-major lower triangle (n(n+1)/2 entries for n element DOFs)
   */
  JaggedArray<Real, 1, 1> _mass_factors;
  /**
   * Right-hand side and solution of the L2 projection of each element, indexed by the contiguous
   * element ID (n entries for n element DOFs)
   */
  ///@{
  JaggedArray<Real, 1, 1> _rhs;
  JaggedArray<Real, 1, 1> _solution;
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION void
AuxKernel::operator()(ElementLoop, const ThreadID tid, const Derived & auxkernel) const
{
  auto elem = kokkosBlockElementID(tid);

  AssemblyDatum datum(elem,
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

  AssemblyDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  auxkernel.computeNodeInternal(auxkernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
AuxKernel::operator()(MassMatrixLoop, const ThreadID tid, const Derived & /* auxkernel */) const
{
  const auto & assembly = kokkosAssembly();
  const auto info = kokkosMesh().getElementInfo(kokkosBlockElementID(tid));
  const auto n = assembly.getNumDofs(info.type, _fe_type);
  const auto & phi = assembly.getPhi(info.subdomain, info.type, _fe_type);

  auto factor = _mass_factors[info.id];
  auto L = &factor[0];

  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = 0; j <= i; ++j)
      L[Utils::packedLowerIndex(i, j)] = 0;

  for (unsigned int qp = 0; qp < assembly.getNumQps(info); ++qp)
  {
    const auto JxW = assembly.getJxW(info, qp);

    for (unsigned int i = 0; i < n; ++i)
    {
      const auto t = JxW * phi(i, qp);

      for (unsigned int j = 0; j <= i; ++j)
        L[Utils::packedLowerIndex(i, j)] += t * phi(j, qp);
    }
  }

  if (n > 1)
    Utils::choleskyFactor(L, n);
}

template <typename Derived>
KOKKOS_FUNCTION void
AuxKernel::computeElementInternal(const Derived & auxkernel, AssemblyDatum & datum) const
{
  auto b = _rhs[datum.elemID()];
  auto x = _solution[datum.elemID()];

  const auto n = datum.n_dofs();

  for (unsigned int i = 0; i < n; ++i)
  {
    x[i] = 0;
    b[i] = 0;
  }

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    const auto value = auxkernel.template computeValue<Derived>(qp, datum);

    for (unsigned int i = 0; i < n; ++i)
      b[i] += datum.JxW(qp) * _test(datum, i, qp) * value;
  }

  auto factor = _mass_factors[datum.elemID()];

  if (n == 1)
    x[0] = b[0] / factor[0];
  else
    Utils::choleskySubstitute(&factor[0], &x[0], &b[0], n);

  setElementSolution(&x[0], datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
AuxKernel::computeNodeInternal(const Derived & auxkernel, AssemblyDatum & datum) const
{
  auto value = auxkernel.template computeValue<Derived>(0, datum);

  setNodeSolution(value, datum);
}

KOKKOS_FUNCTION inline void
AuxKernel::setElementSolution(const Real * const values,
                              const AssemblyDatum & datum,
                              const unsigned int comp) const
{
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto var_num = _kokkos_var.var(comp);
  auto tag = _kokkos_var.tag();
  auto elem = datum.elem().id;

  for (unsigned int i = 0; i < datum.n_dofs(); ++i)
    sys.getVectorDofValue(sys.getElemLocalDofIndex(elem, i, var_num), tag) = values[i];
}

KOKKOS_FUNCTION inline void
AuxKernel::setNodeSolution(const Real value,
                           const AssemblyDatum & datum,
                           const unsigned int comp) const
{
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto var_num = _kokkos_var.var(comp);
  auto tag = _kokkos_var.tag();
  auto node = datum.node();

  sys.getVectorDofValue(sys.getNodeLocalDofIndex(node, 0, var_num), tag) = value;
}

} // namespace Moose::Kokkos
