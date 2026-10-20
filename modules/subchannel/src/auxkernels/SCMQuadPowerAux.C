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

InputParameters
SCMQuadPowerAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes axial power rate (W/m) assigned to the fuel pins in a quadrilateral lattice "
      "arrangement");
  params.addRequiredParam<PostprocessorName>(
      "power", "The postprocessor or Real to use for the total power of the subassembly [W]");
  params.addRequiredParam<std::string>(
      "filename", "name of radial power profile .txt file (should be a single column) [UnitLess].");
  params.addParam<FunctionName>(
      "axial_heat_rate",
      "1.0",
      "Axial shape of the heat rate as a function of the distance from the start of the "
      "heated section [Unitless]. Its magnitude does not matter because each pin is "
      "rescaled to its assigned power");
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
  if (processor_id() > 0)
    return;

  if (!_quadMesh.pinMeshExist())
    mooseError(name(), ": This object requires a pin mesh.");

  auto n_pins = _quadMesh.getNumOfPins();
  // Matrix sizing
  _power_dis.resize(n_pins, 1);
  _power_dis.setZero();

  Real vin;
  std::ifstream inFile;

  inFile.open(_filename);
  if (!inFile)
    mooseError(name(), "unable to open file : ", _filename);

  while (inFile >> vin)
    _numberoflines += 1;

  if (inFile.fail() && !inFile.eof())
    mooseError(name(), " non numerical input at line : ", _numberoflines);

  if (_numberoflines != n_pins)
    mooseError(name(), " Radial profile file doesn't have correct size : ", n_pins);
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
  computeShapeIntegral();
}

void
SCMQuadPowerAux::timestepSetup()
{
  // The axial power shape may depend on time, and the time is advanced before this is called
  computeShapeIntegral();
}

void
SCMQuadPowerAux::computeShapeIntegral()
{
  if (processor_id() > 0)
    return;

  auto n_pins = _quadMesh.getNumOfPins();
  auto nz = _quadMesh.getNumOfAxialCells();
  auto z_grid = _quadMesh.getZGrid();
  auto heated_length = _quadMesh.getHeatedLength();
  auto unheated_length_entry = _quadMesh.getHeatedLengthEntry();

  _shape_integral.resize(n_pins, 1);
  _shape_integral.setZero();
  for (unsigned int iz = 1; iz < nz + 1; iz++)
  {
    // Compute axial location of nodes.
    auto z2 = z_grid[iz];
    auto z1 = z_grid[iz - 1];
    if (MooseUtils::absoluteFuzzyGreaterThan(z2, unheated_length_entry) &&
        MooseUtils::absoluteFuzzyLessThan(z1, unheated_length_entry + heated_length))
    {
      // cycle through pins
      for (unsigned int i_pin = 0; i_pin < n_pins; i_pin++)
      {
        // Evaluate the axial shape at this pin's nodes, where computeValue() assigns q_prime and
        // the solver integrates it, so the correction also holds for shapes that vary between pins
        const Point shift(0, 0, unheated_length_entry);
        auto heat1 =
            _axial_heat_rate.value(_t, Point(*_quadMesh.getPinNode(i_pin, iz - 1) - shift));
        auto heat2 = _axial_heat_rate.value(_t, Point(*_quadMesh.getPinNode(i_pin, iz) - shift));

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

        _shape_integral(i_pin) += (heat1 + heat2) * dz / 2.0;
      }
    }
  }
}

Real
SCMQuadPowerAux::computeValue()
{
  Point p = *_current_node;
  auto heated_length = _quadMesh.getHeatedLength();
  auto unheated_length_entry = _quadMesh.getHeatedLengthEntry();
  Point p1(0, 0, unheated_length_entry);
  Point P = p - p1;

  /// assign power to the nodes located within the heated section
  if (MooseUtils::absoluteFuzzyGreaterEqual(p(2), unheated_length_entry) &&
      MooseUtils::absoluteFuzzyLessEqual(p(2), unheated_length_entry + heated_length))
  {
    auto i_pin = _quadMesh.getPinIndexFromPoint(p);
    // A pin with a zero axial shape receives no power
    if (_shape_integral(i_pin) == 0.0)
      return 0.0;
    // The power postprocessor is a dependency of this object, so it executes first on every
    // execute_on flag they share and holds its current value here
    const auto pin_power = _power * _power_dis(i_pin) / _power_dis.sum();
    // Dividing by the trapezoidal integral of the shape makes the trapezoidal integral of
    // q_prime over the pin, which the subchannel solver uses, equal the pin power
    return pin_power * _axial_heat_rate.value(_t, P) / _shape_integral(i_pin);
  }
  else
    return 0.0;
}
