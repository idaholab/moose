/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticitySlipRate.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<CrystalPlasticitySlipRate>()
{
  InputParameters params = validParams<CrystalPlasticityUOBase>();
  params.addParam<unsigned int>("num_slip_sys_props",
                                0,
                                "Number of slip system specific properties provided in the file "
                                "containing slip system normals and directions");
  params.addParam<std::vector<Real>>("flowprops", "Parameters used in slip rate equations");
  params.addRequiredParam<FileName>("slip_sys_file_name",
                                    "Name of the file containing the slip system");
  params.addParam<FileName>(
      "slip_sys_flow_prop_file_name",
      "",
      "Name of the file containing the values of slip rate equation parameters");
  params.addParam<unsigned int>(
      "num_slip_sys_flowrate_props",
      2,
      "Number of flow rate properties for a slip system"); // Used for reading flow rate parameters
  params.addParam<Real>("slip_incr_tol", 2e-2, "Maximum allowable slip in an increment");
  params.addClassDescription(
      "Crystal plasticity slip rate class.  Override the virtual functions in your class");
  return params;
}

CrystalPlasticitySlipRate::CrystalPlasticitySlipRate(const InputParameters & parameters)
  : CrystalPlasticityUOBase(parameters),
    _num_slip_sys_props(getParam<unsigned int>("num_slip_sys_props")),
    _flowprops(getParam<std::vector<Real>>("flowprops")),
    _slip_sys_file_name(getParam<FileName>("slip_sys_file_name")),
    _slip_sys_flow_prop_file_name(getParam<FileName>("slip_sys_flow_prop_file_name")),
    _num_slip_sys_flowrate_props(getParam<unsigned int>("num_slip_sys_flowrate_props")),
    _slip_incr_tol(getParam<Real>("slip_incr_tol")),
    _mo(_variable_size * LIBMESH_DIM),
    _no(_variable_size * LIBMESH_DIM),
    _crysrot(getMaterialPropertyByName<RankTwoTensor>("crysrot"))
{
  getSlipSystems();
}

void
CrystalPlasticitySlipRate::readFileFlowRateParams()
{
}

void
CrystalPlasticitySlipRate::getFlowRateParams()
{
}

void
CrystalPlasticitySlipRate::getSlipSystems()
{
  Real vec[LIBMESH_DIM];
  std::ifstream fileslipsys;

  MooseUtils::checkFileReadable(_slip_sys_file_name);

  fileslipsys.open(_slip_sys_file_name.c_str());

  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    // Read the slip normal
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      if (!(fileslipsys >> vec[j]))
        mooseError(
            "CrystalPlasticitySlipRate Error: Premature end of file reading slip system file \n");

    // Normalize the vectors
    Real mag;
    mag = Utility::pow<2>(vec[0]) + Utility::pow<2>(vec[1]) + Utility::pow<2>(vec[2]);
    mag = std::sqrt(mag);

    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      _no(i * LIBMESH_DIM + j) = vec[j] / mag;

    // Read the slip direction
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      if (!(fileslipsys >> vec[j]))
        mooseError(
            "CrystalPlasticitySlipRate Error: Premature end of file reading slip system file \n");

    // Normalize the vectors
    mag = Utility::pow<2>(vec[0]) + Utility::pow<2>(vec[1]) + Utility::pow<2>(vec[2]);
    mag = std::sqrt(mag);

    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      _mo(i * LIBMESH_DIM + j) = vec[j] / mag;

    mag = 0.0;
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      mag += _mo(i * LIBMESH_DIM + j) * _no(i * LIBMESH_DIM + j);

    if (std::abs(mag) > 1e-8)
      mooseError("CrystalPlasticitySlipRate Error: Slip direction and normal not orthonormal, "
                 "System number = ",
                 i,
                 "\n");
  }

  fileslipsys.close();
}
