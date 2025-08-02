//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// Torch includes
#include <ATen/ops/from_blob.h>

// MOOSE includes
#include "NEML2Assembly.h"

using namespace libMesh;

registerMooseObject("MooseApp", NEML2Assembly);

InputParameters
NEML2Assembly::validParams()
{
  InputParameters params = ElementUserObject::validParams();

  params.addClassDescription(
      "This user object gathers the JxWxT values from all elements in the assembly and "
      "provides them as a neml2 tensor. This is useful for assembling NEML2 models that "
      "require the JxWxT values for each element.");

  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

NEML2Assembly::NEML2Assembly(const InputParameters & parameters) : ElementUserObject(parameters) {}

void
NEML2Assembly::invalidate()
{
  _up_to_date = false;
}

void
NEML2Assembly::initialize()
{
  if (_up_to_date)
    return;

  _nelem = 0;
  _nqp = 0;
  _moose_JxWxT.clear();
}

void
NEML2Assembly::threadJoin(const UserObject & y)
{
  const auto & other = static_cast<const NEML2Assembly &>(y);
  mooseAssert(_up_to_date == other._up_to_date,
              "NEML2Assembly becomes out of sync with other thread");

  if (_up_to_date)
    return;

  _nelem += other._nelem;
  mooseAssert(_nqp == other._nqp,
              "The number of quadrature points per element must be the same in all threads.");

  _moose_JxWxT.insert(_moose_JxWxT.end(), other._moose_JxWxT.begin(), other._moose_JxWxT.end());
}

void
NEML2Assembly::execute()
{
  if (_up_to_date)
    return;

  _nelem++;

  // number of quadrature points
  if (_nqp != 0 && std::size_t(_nqp) != _q_point.size())
    mooseError("All elements must have the same number of quadrature points per element for all "
               "elements");
  _nqp = _q_point.size();

  // JxWxT
  for (auto qp : index_range(_q_point))
    _moose_JxWxT.push_back(_JxW[qp] * _coord[qp]);
}

void
NEML2Assembly::finalize()
{
  TIME_SECTION("finalize", 1, "Updating FEM assembly for NEML2");

  if (_up_to_date)
    return;

  // sanity checks on sizes
  if (_moose_JxWxT.size() != std::size_t(_nelem * _nqp))
    mooseError("JxWxT size mismatch, expected ", _nelem * _nqp, " but got ", _moose_JxWxT.size());

  // convert gathered data to neml2 tensors (and send to device)
  auto device = _app.getLibtorchDevice();
  _neml2_JxWxT =
      neml2::Tensor(at::from_blob(_moose_JxWxT.data(), {_nelem, _nqp}, torch::kFloat64), 2)
          .to(device);

  // done
  _up_to_date = true;
}

#endif
