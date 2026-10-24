//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMortarSegmentIntegral.h"

#include "AutomaticMortarGeneration.h"
#include "MooseVariableFieldBase.h"
#include "MortarUtils.h"

#include "libmesh/fe.h"
#include "libmesh/fe_dual_coeffs.h"
#include "libmesh/fe_interface.h"
#include "libmesh/kokkos_fe_evaluator.h"
#include "libmesh/quadrature.h"

registerMooseObject("MooseTestApp", KokkosMortarSegmentIntegral);

InputParameters
KokkosMortarSegmentIntegral::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Integrates the product of a variable on the secondary and the "
                             "primary side of a mortar interface over its mortar segments.");
  params.addRequiredParam<BoundaryName>("primary_boundary", "The primary boundary");
  params.addRequiredParam<BoundaryName>("secondary_boundary", "The secondary boundary");
  params.addRequiredParam<SubdomainName>("primary_subdomain",
                                         "The lower-dimensional primary subdomain");
  params.addRequiredParam<SubdomainName>("secondary_subdomain",
                                         "The lower-dimensional secondary subdomain");
  params.addRequiredCoupledVar(
      "variable", "The variable defined on the lower-dimensional secondary and primary blocks");
  params.addParam<Real>("tolerance",
                        1e-12,
                        "Relative tolerance on the agreement of the integral evaluated on the "
                        "device with the integral evaluated through libMesh on the host");
  return params;
}

KokkosMortarSegmentIntegral::KokkosMortarSegmentIntegral(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _amg(_fe_problem.getMortarInterface(
        std::make_pair(
            _fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("primary_boundary")),
            _fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("secondary_boundary"))),
        std::make_pair(
            _fe_problem.mesh().getSubdomainID(getParam<SubdomainName>("primary_subdomain")),
            _fe_problem.mesh().getSubdomainID(getParam<SubdomainName>("secondary_subdomain"))),
        false)),
    _moose_var(*getFieldVar("variable", 0)),
    _var(kokkosCoupledValue("variable").variable()),
    _fe_type(_moose_var.feType())
{
}

void
KokkosMortarSegmentIntegral::compute()
{
  _fe_type_id = kokkosAssembly().getFETypeID(_fe_type);

  const auto & mesh = _fe_problem.mesh();
  const auto & kokkos_mesh = *mesh.getKokkosMesh();
  const auto & qrule = _fe_problem.assembly(0, 0).qRuleMortar();
  const auto secondary_sub_elem_index =
      _amg.dim() == 3 ? _amg.mortarSegmentMesh().get_elem_integer_index("secondary_sub_elem") : 0;
  const auto primary_sub_elem_index =
      _amg.dim() == 3 ? _amg.mortarSegmentMesh().get_elem_integer_index("primary_sub_elem") : 0;

  std::vector<ContiguousElementID> secondary_elem, primary_elem;
  std::vector<libMesh::ElemType> secondary_type, primary_type;
  std::vector<Point> secondary_xi, primary_xi;
  std::vector<Real> JxW;

  // The host reference evaluates the variable through libMesh at the same reference points
  const auto & solution = *_moose_var.sys().currentSolution();
  const auto & dof_map = _moose_var.sys().dofMap();
  std::vector<dof_id_type> dof_indices;
  auto host_value = [&](const Elem & elem, const Point & xi)
  {
    dof_map.dof_indices(&elem, dof_indices, _moose_var.number());
    Real value = 0;
    for (const auto i : index_range(dof_indices))
      value += solution(dof_indices[i]) * libMesh::FEInterface::shape(_fe_type, &elem, i, xi);
    return value;
  };

  _host_integral = 0;

  // libMesh computes the dual basis coefficients of each secondary element from its segment
  // quadrature, which they are checked against
  std::unique_ptr<libMesh::FEBase> fe(libMesh::FEBase::build(_amg.dim() - 1, _fe_type));
  std::vector<dof_id_type> secondary_qp_begin = {0};
  _host_dual_coeff.clear();

  for (const auto & [secondary_id, msm_elems] : _amg.secondariesToMortarSegments())
  {
    // The map also lists secondary elements of other processes, which a distributed mesh need not
    // hold
    const auto secondary = mesh.getMesh().query_elem_ptr(secondary_id);
    if (!secondary || secondary->processor_id() != processor_id())
      continue;

    std::vector<Point> secondary_pts;
    std::vector<Real> secondary_JxW;

    for (const auto msm_elem : msm_elems)
    {
      _fe_problem.reinitMortarElem(msm_elem, 0);
      const auto & msinfo = libmesh_map_find(_amg.mortarSegmentMeshElemToInfo(), msm_elem);
      const auto & JxW_msm = _fe_problem.assembly(0, 0).jxWMortar();

      std::vector<Point> xi1, xi2;
      if (msm_elem->dim() == 1)
        for (const auto qp : make_range(qrule->n_points()))
        {
          const Real eta = qrule->qp(qp)(0);
          xi1.push_back(0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b);
          xi2.push_back(0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b);
        }
      else if (_amg.mortar3DQpMapping() == Mortar3DQuadraturePointMapping::REFERENCE_INTERPOLATION)
        Moose::Mortar::mapQPoints3dFromReference(
            *msm_elem, _amg.mortarSegmentReferencePoints(*msm_elem), *qrule, xi1, xi2);
      else
      {
        Moose::Mortar::projectQPoints3d(
            msm_elem, msinfo.secondary_elem, secondary_sub_elem_index, *qrule, xi1);
        Moose::Mortar::projectQPoints3d(
            msm_elem, msinfo.primary_elem, primary_sub_elem_index, *qrule, xi2);
      }

      for (const auto qp : make_range(qrule->n_points()))
      {
        secondary_elem.push_back(kokkos_mesh.getContiguousElementID(msinfo.secondary_elem));
        primary_elem.push_back(kokkos_mesh.getContiguousElementID(msinfo.primary_elem));
        secondary_type.push_back(msinfo.secondary_elem->type());
        primary_type.push_back(msinfo.primary_elem->type());
        secondary_xi.push_back(xi1[qp]);
        primary_xi.push_back(xi2[qp]);
        JxW.push_back(JxW_msm[qp]);

        _host_integral += JxW_msm[qp] * host_value(*msinfo.secondary_elem, xi1[qp]) *
                          host_value(*msinfo.primary_elem, xi2[qp]);

        secondary_pts.push_back(xi1[qp]);
        secondary_JxW.push_back(JxW_msm[qp]);
      }
    }

    secondary_qp_begin.push_back(JxW.size());

    fe->reinit_dual_shape_coeffs(secondary, secondary_pts, secondary_JxW);
    const auto & coeff = fe->get_dual_coeff();
    mooseAssert(coeff.m() <= max_shapes, "Too many shape functions");
    for (const auto i : make_range(max_shapes))
      for (const auto j : make_range(max_shapes))
        _host_dual_coeff.push_back(i < coeff.m() && j < coeff.n() ? coeff(i, j) : 0);
  }

  const auto n = JxW.size();
  _secondary_elem.create(n);
  _primary_elem.create(n);
  _secondary_type.create(n);
  _primary_type.create(n);
  _secondary_xi.create(n);
  _primary_xi.create(n);
  _JxW.create(n);

  for (const auto qp : make_range(n))
  {
    _secondary_elem[qp] = secondary_elem[qp];
    _primary_elem[qp] = primary_elem[qp];
    _secondary_type[qp] = secondary_type[qp];
    _primary_type[qp] = primary_type[qp];
    _secondary_xi[qp] = secondary_xi[qp];
    _primary_xi[qp] = primary_xi[qp];
    _JxW[qp] = JxW[qp];
  }

  _secondary_elem.copyToDevice();
  _primary_elem.copyToDevice();
  _secondary_type.copyToDevice();
  _primary_type.copyToDevice();
  _secondary_xi.copyToDevice();
  _primary_xi.copyToDevice();
  _JxW.copyToDevice();

  _integral = 0;
  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<Moose::Kokkos::ThreadID>> policy(0, n);
  ::Kokkos::parallel_reduce(policy, *this, _integral);
  ::Kokkos::fence();

  const auto n_secondary = secondary_qp_begin.size() - 1;
  _secondary_qp_begin.create(secondary_qp_begin.size());
  for (const auto i : index_range(secondary_qp_begin))
    _secondary_qp_begin[i] = secondary_qp_begin[i];
  _secondary_qp_begin.copyToDevice();

  _dual_coeff.create(n_secondary * max_shapes * max_shapes);
  _dual_coeff = 0;

  ::Kokkos::RangePolicy<ExecSpace, DualCoeffLoop, ::Kokkos::IndexType<Moose::Kokkos::ThreadID>>
      dual_policy(0, n_secondary);
  ::Kokkos::parallel_for(dual_policy, *this);
  ::Kokkos::fence();

  _dual_coeff.copyToHost();
}

KOKKOS_FUNCTION void
KokkosMortarSegmentIntegral::operator()(DualCoeffLoop,
                                        const Moose::Kokkos::ThreadID secondary) const
{
  const auto begin = _secondary_qp_begin[secondary];
  const auto n_qp = _secondary_qp_begin[secondary + 1] - begin;
  const auto elem = _secondary_elem[begin];
  const auto type = _secondary_type[begin];
  const auto n_shapes =
      kokkosAssembly().getNumDofs(kokkosMesh().getElementInfo(elem).type, _fe_type_id);
  const libMesh::FEShapeKey key{_fe_type.family, type, _fe_type.order};

  Real A[max_shapes][max_shapes], D[max_shapes][max_shapes], rhs[max_shapes], x[max_shapes];
  auto matrix = [](Real(&M)[max_shapes][max_shapes])
  {
    return [&M](const unsigned int i, const unsigned int j) -> Real & { return M[i][j]; };
  };
  auto vector = [](Real(&v)[max_shapes])
  {
    return [&v](const unsigned int i) -> Real & { return v[i]; };
  };
  auto A_ref = matrix(A);
  auto D_ref = matrix(D);
  auto rhs_ref = vector(rhs);
  auto x_ref = vector(x);

  const auto JxW = [&](const unsigned int qp) { return _JxW[begin + qp]; };
  const auto phi = [&](const unsigned int i, const unsigned int qp)
  {
    const auto & xi = _secondary_xi[begin + qp];
    return libMesh::Kokkos::shape(key, i, xi(0), xi(1), xi(2));
  };
  auto coeff = [&](const unsigned int i, const unsigned int j) -> Real &
  { return _dual_coeff[(secondary * max_shapes + i) * max_shapes + j]; };

  libMesh::compute_dual_coeffs(
      _fe_type.family, type, n_shapes, n_qp, JxW, phi, A_ref, D_ref, rhs_ref, x_ref, coeff);
}

KOKKOS_FUNCTION void
KokkosMortarSegmentIntegral::operator()(const Moose::Kokkos::ThreadID qp, Real & sum) const
{
  const auto & mesh = kokkosMesh();
  const auto & sys = kokkosSystem(_var.sys());
  const auto var = _var.var();
  const auto tag = _var.tag();

  auto value = [&](const ContiguousElementID elem,
                   const libMesh::ElemType type,
                   const Moose::Kokkos::Real3 & xi) -> Real
  {
    const auto n_dofs = kokkosAssembly().getNumDofs(mesh.getElementInfo(elem).type, _fe_type_id);
    const libMesh::FEShapeKey key{_fe_type.family, type, _fe_type.order};

    Real u = 0;
    for (unsigned int i = 0; i < n_dofs; ++i)
      u += sys.getVectorDofValue(sys.getElemLocalDofIndex(elem, i, var), tag) *
           libMesh::Kokkos::shape(key, i, xi(0), xi(1), xi(2));
    return u;
  };

  sum += _JxW[qp] * value(_secondary_elem[qp], _secondary_type[qp], _secondary_xi[qp]) *
         value(_primary_elem[qp], _primary_type[qp], _primary_xi[qp]);
}

void
KokkosMortarSegmentIntegral::finalize()
{
  Real max_dual_error = 0, max_dual_coeff = 0;
  for (const auto i : index_range(_host_dual_coeff))
  {
    max_dual_error = std::max(max_dual_error, std::abs(_dual_coeff[i] - _host_dual_coeff[i]));
    max_dual_coeff = std::max(max_dual_coeff, std::abs(_host_dual_coeff[i]));
  }
  comm().max(max_dual_error);
  comm().max(max_dual_coeff);

  if (max_dual_error > getParam<Real>("tolerance") * std::max(max_dual_coeff, Real(1)))
    mooseError("The dual basis coefficients computed on the device differ from those computed by "
               "libMesh on the host by up to ",
               max_dual_error,
               ".");

  gatherSum(_integral);
  gatherSum(_host_integral);

  if (std::abs(_integral - _host_integral) >
      getParam<Real>("tolerance") * std::max(std::abs(_host_integral), Real(1)))
    mooseError("The mortar segment integral evaluated on the device, ",
               _integral,
               ", differs from the integral evaluated through libMesh on the host, ",
               _host_integral,
               ".");
}
