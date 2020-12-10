/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#include "ACPolycrystalDislocationEnergy.h"
#include "GrainDataTracker.h"

registerMooseObject("MarmotApp", ACPolycrystalDislocationEnergy);

template <>
InputParameters
validParams<ACPolycrystalDislocationEnergy>()
{
  InputParameters params = validParams<ACGrGrBase>();

  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  params.addRequiredParam<unsigned int>("op_index", "the index for the current order parameter");

  params.addClassDescription(
      "polycrystalline Allen-Cahn kernel for dislocation energy driving force");

  return params;
}

ACPolycrystalDislocationEnergy::ACPolycrystalDislocationEnergy(const InputParameters & parameters)
  : ACGrGrBase(parameters),
    _G(getMaterialProperty<Real>("shear_modulus")),
    _b(getMaterialProperty<Real>("burgers_vector")),
    _dislocation_density(getMaterialProperty<Real>("dislocation_density")),
    _sum_h_OP(getMaterialProperty<Real>("sum_h_OP")),
    _grain_tracker(getUserObject<GrainDataTracker<Real>>("grain_tracker")),
    _op_index(getParam<unsigned int>("op_index")),
    _grain_disloc_data(getMaterialProperty<std::vector<Real>>("grain_disloc_data"))
{
}

Real
ACPolycrystalDislocationEnergy::computeDFDOP(PFFunctionType type)
{
  Real k = 0.5 * _G[_qp] * _b[_qp] * _b[_qp];
  Real rho_i = 0;
  Real dislocation_density = 0;

  // derivative of the interpolation function
  Real dh_dOP = 2 * _u[_qp];

  // get the vector that maps active order parameters to grain ids
  const auto & op_to_grain = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  //  //find out of this kernel's order parameter is active at this point and if so, get its grain ID
  const auto grain_id = op_to_grain[_op_index];

  if (grain_id != FeatureFloodCount::invalid_id && grain_id < _grain_tracker.getTotalFeatureCount())
  {
    rho_i = _grain_disloc_data[_qp][grain_id];
    dislocation_density = _dislocation_density[_qp];
  }

  switch (type)
  {
    case Residual:
    {
      Real test = k * dh_dOP * (rho_i - dislocation_density) / _sum_h_OP[_qp];

      if (std::isnormal(test))
        return test;
      else
        return 0;
    }
    case Jacobian:
    {
      return 0;
    }
  }
  mooseError("error in ACPolycrystalDislocationDensity");
}
