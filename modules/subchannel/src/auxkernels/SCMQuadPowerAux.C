//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMQuadPowerAux.h"
#include "Function.h"
#include "QuadSubChannelMesh.h"
#include "SCM.h"

registerMooseObject("SubChannelApp", SCMQuadPowerAux);
registerMooseObjectRenamed("SubChannelApp", QuadPowerAux, "06/30/2025 24:00", SCMQuadPowerAux);

InputParameters
SCMQuadPowerAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes axial power rate (W/m) that goes into the subchannel cells "
      "or is assigned to the fuel pins, in a quadrilateral lattice arrangement");
  params.addRequiredParam<PostprocessorName>(
      "power", "The postprocessor or Real to use for the total power of the subassembly [W]");
  params.addRequiredParam<std::string>(
      "filename", "name of radial power profile .txt file (should be a single column) [UnitLess].");
  params.addParam<FunctionName>("axial_heat_rate",
                                "1.0",
                                "user provided normalized function of axial heat rate [Unitless]. "
                                "The integral over pin length should equal the heated length");
  return params;
}

SCMQuadPowerAux::SCMQuadPowerAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _quadMesh(SCM::getConstMesh<QuadSubChannelMesh>(_mesh)),
    _power(getPostprocessorValue("power")),
    _numberoflines(0),
    _filename(getParam<std::string>("filename")),
    _axial_heat_rate(getFunction("axial_heat_rate"))
{
  auto nx = _quadMesh.getNx();
  auto ny = _quadMesh.getNy();
  // Matrix sizing
  _power_dis.resize((ny - 1) * (nx - 1), 1);
  _power_dis.setZero();
  _pin_power_correction.resize((ny - 1) * (nx - 1), 1);
  _pin_power_correction.setOnes();

  Real vin;
  std::ifstream inFile;

  inFile.open(_filename);
  if (!inFile)
    mooseError(name(), "unable to open file : ", _filename);

  while (inFile >> vin)
    _numberoflines += 1;

  if (inFile.fail() && !inFile.eof())
    mooseError(name(), " non numerical input at line : ", _numberoflines);

  if (_numberoflines != (ny - 1) * (nx - 1))
    mooseError(name(), " Radial profile file doesn't have correct size : ", (ny - 1) * (nx - 1));
  inFile.close();

  inFile.open(_filename);
  int i(0);
  while (inFile >> vin)
  {
    _power_dis(i, 0) = vin;
    i++;
  }
  inFile.close();
  _console << " Power distribution matrix :\n" << _power_dis << " \n";
}

void
SCMQuadPowerAux::initialSetup()
{
  auto nx = _quadMesh.getNx();
  auto ny = _quadMesh.getNy();
  auto n_pins = (nx - 1) * (ny - 1);
  auto nz = _quadMesh.getNumOfAxialCells();
  auto z_grid = _quadMesh.getZGrid();
  auto heated_length = _quadMesh.getHeatedLength();
  auto unheated_length_entry = _quadMesh.getHeatedLengthEntry();
  auto sum = _power_dis.sum();

  // full (100%) power of one pin [W]
  auto fpin_power = _power / sum;
  // actual pin power [W]
  _ref_power = _power_dis * fpin_power;
  // Convert the actual pin power to a linear heat rate [W/m]
  _ref_qprime = _ref_power / heated_length;

  _estimate_power.resize(n_pins, 1);
  _estimate_power.setZero();
  for (unsigned int iz = 1; iz < nz + 1; iz++)
  {
    // Compute axial location of nodes.
    auto z2 = z_grid[iz];
    auto z1 = z_grid[iz - 1];
    Point p1(0, 0, z1 - unheated_length_entry);
    Point p2(0, 0, z2 - unheated_length_entry);
    auto heat1 = _axial_heat_rate.value(_t, p1);
    auto heat2 = _axial_heat_rate.value(_t, p2);
    if (MooseUtils::absoluteFuzzyGreaterThan(z2, unheated_length_entry) &&
        MooseUtils::absoluteFuzzyLessThan(z1, unheated_length_entry + heated_length))
    {
      // cycle through pins
      for (unsigned int i_pin = 0; i_pin < n_pins; i_pin++)
      {
        // Compute the height of this element.
        auto dz = z2 - z1;

        // calculation of power for the first heated segment if nodes don't align
        if (MooseUtils::absoluteFuzzyGreaterThan(z2, unheated_length_entry) &&
            MooseUtils::absoluteFuzzyLessThan(z1, unheated_length_entry))
        {
          heat1 = 0.0;
        }

        // calculation of power for the last heated segment if nodes don't align
        if (MooseUtils::absoluteFuzzyGreaterThan(z2, unheated_length_entry + heated_length) &&
            MooseUtils::absoluteFuzzyLessThan(z1, unheated_length_entry + heated_length))
        {
          heat2 = 0.0;
        }

        _estimate_power(i_pin) += _ref_qprime(i_pin) * (heat1 + heat2) * dz / 2.0;
      }
    }
  }

  // if a Pin has zero power (_ref_qprime(i_pin) = 0) then I need to avoid dividing by zero. I
  // divide by a wrong non-zero number which is not correct but this error doesn't mess things
  // cause _ref_qprime(i_pin) = 0.0
  auto total_power = 0.0;
  for (unsigned int i_pin = 0; i_pin < n_pins; i_pin++)
  {
    total_power += _estimate_power(i_pin);
    if (_estimate_power(i_pin) == 0.0)
      _estimate_power(i_pin) = 1.0;
  }
  // We need to correct the linear power assigned to the nodes of each pin
  // so that the total power calculated  by the trapezoidal rule agrees with the power assigned
  // by the user.
  _pin_power_correction = _ref_power.cwiseQuotient(_estimate_power);
  _pin_power_correction = _ref_power.cwiseQuotient(_estimate_power);
  _console << "###########################################" << std::endl;
  _console << "Total power estimation: " << total_power << " [W] " << std::endl;
}

Real
SCMQuadPowerAux::computeValue()
{
  Point p = *_current_node;
  auto heated_length = _quadMesh.getHeatedLength();
  auto unheated_length_entry = _quadMesh.getHeatedLengthEntry();
  Point p1(0, 0, unheated_length_entry);
  Point P = p - p1;
  auto pin_mesh_exist = _quadMesh.pinMeshExist();

  /// assign power to the nodes located within the heated section
  if (MooseUtils::absoluteFuzzyGreaterEqual(p(2), unheated_length_entry) &&
      MooseUtils::absoluteFuzzyLessEqual(p(2), unheated_length_entry + heated_length))
  {
    if (pin_mesh_exist)
    {
      // project axial heat rate on pins
      auto i_pin = _quadMesh.getPinIndexFromPoint(p);
      return _ref_qprime(i_pin) * _pin_power_correction(i_pin) * _axial_heat_rate.value(_t, P);
    }
    else
    {
      // project axial heat rate on subchannels
      auto i_ch = _quadMesh.getSubchannelIndexFromPoint(p);
      // if we are adjacent to the heated part of the fuel Pin
      auto heat_rate = 0.0;
      for (auto i_pin : _quadMesh.getChannelPins(i_ch))
      {
        heat_rate += 0.25 * _ref_qprime(i_pin) * _pin_power_correction(i_pin) *
                     _axial_heat_rate.value(_t, P);
      }
      return heat_rate;
    }
  }
  else
    return 0.0;
}
