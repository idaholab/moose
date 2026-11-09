//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriPowerAux.h"
#include "Function.h"
#include "Receiver.h"
#include "TriSubChannelMesh.h"
#include "SCM.h"

registerMooseObject("SubChannelApp", SCMTriPowerAux);

InputParameters
SCMTriPowerAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes axial power rate (W/m) assigned to the fuel pins in a triangular lattice "
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
  params.addParam<bool>(
      "verbose", false, "Print the per-pin power correction every time it is computed");
  return params;
}

SCMTriPowerAux::SCMTriPowerAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _triMesh(SCM::getConstMesh<TriSubChannelMesh>(_mesh)),
    _power(getPostprocessorValue("power")),
    _numberoflines(0),
    _filename(getParam<std::string>("filename")),
    _axial_heat_rate(getFunction("axial_heat_rate")),
    _verbose(getParam<bool>("verbose"))
{
  if (processor_id() > 0)
    return;
  if (!_triMesh.pinMeshExist())
    mooseError(name(), ": This object requires a pin mesh.");

  auto n_pins = _triMesh.getNumOfPins();
  // Matrix sizing
  _power_dis.resize(n_pins, 1);
  _power_dis.setZero();

  Real vin;
  std::ifstream inFile;

  inFile.open(_filename);
  if (!inFile)
    mooseError(name(), ": Unable to open file: ", _filename);

  while (inFile >> vin)
    _numberoflines += 1;

  if (inFile.fail() && !inFile.eof())
    mooseError(name(), ": Non numerical input at line: ", _numberoflines);

  if (_numberoflines != n_pins)
    mooseError(name(), ": Radial profile file doesn't have correct size: ", n_pins);
  inFile.close();

  inFile.open(_filename);
  int i = 0;
  while (inFile >> vin)
  {
    _power_dis(i, 0) = vin;
    i++;
  }
  inFile.close();
}

void
SCMTriPowerAux::initialSetup()
{
  // q_prime uses the value the power postprocessor holds when this object executes, so it is
  // current only if the postprocessor executes before this object on the same flags. A
  // Receiver is filled by a transfer instead, so its execute_on does not set when it changes.
  if (!isDefaultPostprocessorValue("power") &&
      !dynamic_cast<const Receiver *>(&getUserObjectBase("power")))
  {
    const auto & power_pp = getUserObjectBase("power");
    if (!power_pp.getExecuteOnEnum().isValueSet(getExecuteOnEnum()))
      paramWarning("power",
                   "The power postprocessor does not execute on every execute_on flag of this "
                   "object, so q_prime may use a lagged power");
    if (power_pp.getParam<bool>("force_postaux"))
      paramWarning("power",
                   "The power postprocessor sets force_postaux, so it executes after this object "
                   "and q_prime uses a lagged power");
  }

  computeShapeIntegral();
}

void
SCMTriPowerAux::timestepSetup()
{
  // The axial power shape may depend on time, and the time is advanced before this is called
  computeShapeIntegral();
}

void
SCMTriPowerAux::computeShapeIntegral()
{
  if (processor_id() > 0)
    return;

  auto n_pins = _triMesh.getNumOfPins();
  auto nz = _triMesh.getNumOfAxialCells();
  auto z_grid = _triMesh.getZGrid();
  auto heated_length = _triMesh.getHeatedLength();
  auto unheated_length_entry = _triMesh.getHeatedLengthEntry();

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
        auto heat1 = _axial_heat_rate.value(_t, Point(*_triMesh.getPinNode(i_pin, iz - 1) - shift));
        auto heat2 = _axial_heat_rate.value(_t, Point(*_triMesh.getPinNode(i_pin, iz) - shift));

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

  if (_verbose)
  {
    // The correction is the factor between q_prime and the uncorrected average linear heat
    // rate of each pin, the pin power over the heated length
    Eigen::MatrixXd correction = Eigen::MatrixXd::Zero(n_pins, 1);
    for (const auto i_pin : make_range(n_pins))
      if (_shape_integral(i_pin) != 0.0)
        correction(i_pin) = heated_length / _shape_integral(i_pin);
    _console << "###########################################" << std::endl;
    _console << "Aux Power correction vector :\n" << correction << " \n";
  }
}

Real
SCMTriPowerAux::computeValue()
{
  Point p = *_current_node;
  auto heated_length = _triMesh.getHeatedLength();
  auto unheated_length_entry = _triMesh.getHeatedLengthEntry();
  Point p1(0, 0, unheated_length_entry);
  Point P = p - p1;

  /// assign power to the nodes located within the heated section
  if (MooseUtils::absoluteFuzzyGreaterEqual(p(2), unheated_length_entry) &&
      MooseUtils::absoluteFuzzyLessEqual(p(2), unheated_length_entry + heated_length))
  {
    auto i_pin = _triMesh.getPinIndexFromPoint(p);
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
