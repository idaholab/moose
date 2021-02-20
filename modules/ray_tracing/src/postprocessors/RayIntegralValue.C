//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RayIntegralValue.h"

// Local includes
#include "IntegralRayKernel.h"
#include "RayTracingStudy.h"

registerMooseObject("RayTracingApp", RayIntegralValue);

InputParameters
RayIntegralValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription("Obtains the integrated value accumulated into a Ray from an "
                             "IntegralRayKernel-derived class.");

  params.addRequiredParam<std::string>("ray",
                                       "Name of the Ray to get the final integral value from");
  params.addRequiredParam<std::string>(
      "ray_kernel",
      "The name of the IntegralRayKernel-derived RayKernel to obtain the integral value of");

  return params;
}

RayIntegralValue::RayIntegralValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _study(nullptr)
{
}

void
RayIntegralValue::initialize()
{
  // Look for the IntegralRayKernel by the name provided by the user
  const IntegralRayKernel * integral_ray_kernel = nullptr;
  std::vector<RayKernelBase *> rks;
  _fe_problem.theWarehouse().query().condition<AttribSystem>("RayKernel").queryInto(rks);
  for (const RayKernelBase * rk : rks)
    if (rk->name() == getParam<std::string>("ray_kernel"))
    {
      integral_ray_kernel = dynamic_cast<const IntegralRayKernel *>(rk);
      if (!integral_ray_kernel)
        mooseError(rk->type(), " is not derived from an IntegralRayKernel.");
      _study = &rk->study();
      break;
    }

  // Didn't find one
  if (!integral_ray_kernel)
    paramError("ray_kernel",
               "The RayKernel by the name '",
               getParam<std::string>("ray_kernel"),
               "' was not found.");

  // This requires that our studies use Ray name registration
  if (!_study->useRayRegistration())
    mooseError("Cannot be used because the supplied ",
               _study->type(),
               " does not have Ray registration enabled.\n\nThis is controlled by the "
               "RayTracingStudy private param '_use_ray_registration'.");
  // And also that the Rays are banked, otherwise we can't grab the values
  if (!_study->bankRaysOnCompletion())
    mooseError("Cannot be used because the supplied ",
               _study->type(),
               " does not bank Rays on completion.\n\nThis is controlled by the RayTracingStudy "
               "private param '_bank_rays_on_completion'.");

  // Get the Ray ID
  const auto & ray_name = getParam<std::string>("ray");
  _ray_id = _study->registeredRayID(ray_name, /* graceful = */ true);
  if (_ray_id == Ray::INVALID_RAY_ID)
    paramError("ray", "Could not find a Ray named '", ray_name, "'");

  // Get the index for the data on the ray requested
  _ray_data_index = _study->getRayDataIndex(integral_ray_kernel->integralRayDataName());
}

Real
RayIntegralValue::getValue()
{
  // This gathers the value from the proc that killed the Ray we're looking for
  return _study->getBankedRayData(_ray_id, _ray_data_index);
}
