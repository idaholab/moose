//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// NEML2 includes
#include "neml2/tensors/functions/stack.h"
#include "neml2/tensors/functions/sum.h"

// MOOSE includes
#include "NEML2StressDivergenceRZ.h"

registerMooseObject("SolidMechanicsApp", NEML2StressDivergenceRZ);

InputParameters
NEML2StressDivergenceRZ::validParams()
{
  InputParameters params = NEML2StressDivergence::validParams();
  params.addClassDescription(
      "This user object calculates the stress divergence in axisymmetric (RZ) coordinates for a "
      "given set of displacement variables and assembles the contribution into the residual "
      "vector. Compared to the Cartesian weak form, the radial residual additionally contains "
      "the hoop term $\\psi \\sigma_{\\theta\\theta} / r$.");
  return params;
}

NEML2StressDivergenceRZ::NEML2StressDivergenceRZ(const InputParameters & parameters)
  : NEML2StressDivergence(parameters), _radial_coord(_subproblem.getAxisymmetricRadialCoord())
{
  if (_ndisp != 2)
    paramError(
        "displacements", "exactly 2 displacement variables (radial, axial) required, got ", _ndisp);

  _test_r = &_fe.getPhi(_disp_vars[_radial_coord]);
}

void
NEML2StressDivergenceRZ::forward()
{
  if (!_constitutive.outputReady())
    return;

  torch::InferenceMode guard;

  const bool full_stress = _stress.base_dim() == 2;

  auto dphix = neml2::Tensor(*_grad_test_x, 3);
  auto dphiy = neml2::Tensor(*_grad_test_y, 3);
  auto dphiz = neml2::Tensor::zeros_like(dphix);
  auto dphi = neml2::base_stack({dphix, dphiy, dphiz}, 0); // (nelem, ndofe, nqp; 3, 3)
  // symmetric small/Cauchy stress (SR2, Mandel) or full non-symmetric stress such as PK1 (R2)
  auto stress = (full_stress ? neml2::R2(_stress) : neml2::R2(neml2::SR2(_stress)))
                    .dynamic_unsqueeze(-2); // (nelem, 1, nqp; 3, 3)

  // weak form
  auto re_qp = neml2::base_sum(dphi * neml2::Tensor(stress), -1); // (nelem, ndofe, nqp; 3)

  // hoop term on the radial component: psi * stress_theta_theta / r. The volume element (JxWxT
  // below) already contains the coordinate transformation factor 2*pi*r.
  auto phir = neml2::Tensor(*_test_r, 3); // (nelem, ndofe, nqp;)
  auto stt = (full_stress ? neml2::Tensor(_stress).base_index({2, 2})
                          : neml2::Tensor(_stress).base_index({2}))
                 .dynamic_unsqueeze(-2); // (nelem, 1, nqp;)
  auto r = _neml2_assembly.qPoints()
               .base_index({_radial_coord})
               .dynamic_unsqueeze(-2); // (nelem, 1, nqp;)
  re_qp.base_index_put_({_radial_coord}, re_qp.base_index({_radial_coord}) + phir * stt / r);

  // element integration
  auto JxWxT =
      _neml2_assembly.JxWxT().dynamic_unsqueeze(-2).base_unsqueeze(0); // (nelem, 1, nqp; 1)
  auto re = neml2::dynamic_sum(re_qp * JxWxT, -1)
                .base_index({neml2::indexing::Slice(0, _ndisp)}); // (nelem, ndofe; ndisp)

  // assemble residual
  for (auto i : make_range(_ndisp))
  {
    auto re_i = re.base_index({i}).contiguous().cpu();
    const auto & dofmap_i = _fe.getGlobalDofMap(_disp_vars[i]);
    _residual->add_vector(re_i.data_ptr<Real>(), dofmap_i);
  }
  _residual->close();
}

#endif
