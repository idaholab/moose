//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <iostream>
#include <iomanip>

#include "MooseObject.h"

#include "MutableCoefficientsInterface.h"

InputParameters
MutableCoefficientsInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addClassDescription("This interface universalizes the communication standards for "
                             "array-based coefficient transfers.");

  params.addParam<bool>("print_when_set", false, "Print the array of coefficients when set");

  return params;
}

MutableCoefficientsInterface::MutableCoefficientsInterface(const MooseObject * moose_object,
                                                           const InputParameters & parameters)
  : Restartable(moose_object->getMooseApp(),
                moose_object->name() + "_coefs",
                "MutableCoefficientsInterface",
                moose_object->parameters().get<THREAD_ID>("_tid")),
    _characteristics(declareRestartableData<std::vector<std::size_t>>("characteristics")),
    _coefficients(declareRestartableData<std::vector<Real>>("coefficients")),
    _enforce_size(false),
    _print_coefficients(parameters.get<bool>("print_when_set")),
    _console(moose_object->_console)
{
}

Real
MutableCoefficientsInterface::operator[](std::size_t index) const
{
  return _coefficients[index];
}

const std::vector<std::size_t> &
MutableCoefficientsInterface::getCharacteristics() const
{
  return _characteristics;
}

const std::vector<Real> &
MutableCoefficientsInterface::getCoefficients() const
{
  return _coefficients;
}

std::vector<Real> &
MutableCoefficientsInterface::getCoefficients()
{
  return _coefficients;
}

std::string
MutableCoefficientsInterface::getCoefficientsTable() const
{
  std::stringbuf string;
  std::ostream table(&string);

  table << *this;

  return string.str();
}

std::size_t
MutableCoefficientsInterface::getSize() const
{
  return _coefficients.size();
}

bool
MutableCoefficientsInterface::isCompatibleWith(const MutableCoefficientsInterface & other) const
{
  // Check the coefficient sizes if requested
  if ((_enforce_size && other._enforce_size) && getSize() != other.getSize())
    return false;

  // Check the size of the characteristics array
  if (_characteristics.size() != other._characteristics.size())
    return false;

  // Check the values of the characteristics array
  for (std::size_t i = 0; i < _characteristics.size(); ++i)
    if (_characteristics[i] != other._characteristics[i])
      return false;

  return true;
}

bool
MutableCoefficientsInterface::isSizeEnforced() const
{
  return _enforce_size;
}

void
MutableCoefficientsInterface::enforceSize(bool enforce)
{
  _enforce_size = enforce;
}

void
MutableCoefficientsInterface::importCoefficients(const MutableCoefficientsInterface & other)
{
  if (!isCompatibleWith(other))
    mooseError("Cannot import coefficients from incompatible MutableCoefficientsInterface");

  _coefficients = other._coefficients;

  if (_print_coefficients)
    _console << *this;

  coefficientsChanged();
}

void
MutableCoefficientsInterface::resize(std::size_t size, Real fill, bool fill_out_to_size)
{
  if (size != _coefficients.size())
  {
    if (_enforce_size &&
        (size > _coefficients.size() || (size < _coefficients.size() && !fill_out_to_size)))
      mooseError("Cannot resize coefficient array with size enforcement enabled.");

    _coefficients.resize(size, fill);

    if (_print_coefficients)
      _console << *this;

    coefficientsChanged();
  }
}

void
MutableCoefficientsInterface::setCharacteristics(
    const std::vector<std::size_t> & new_characteristics)
{
  _characteristics = new_characteristics;
}

void
MutableCoefficientsInterface::setCoefficients(const std::vector<Real> & new_coefficients)
{
  if (_enforce_size && new_coefficients.size() != _coefficients.size())
    mooseError("Cannon assigned a coefficient array with differing size when size enforcement is "
               "enabled.");

  _coefficients = new_coefficients;

  if (_print_coefficients)
    _console << *this;

  coefficientsChanged();
}

void
MutableCoefficientsInterface::setCoefficients(std::vector<Real> && dropin_coefficients)
{
  if (_enforce_size && dropin_coefficients.size() != _coefficients.size())
    mooseError("Cannon assigned a coefficient array with differing size when size enforcement is "
               "enabled.");

  _coefficients = dropin_coefficients;

  if (_print_coefficients)
    _console << *this;

  coefficientsChanged();
}

std::ostream &
operator<<(std::ostream & stream, const MutableCoefficientsInterface & me)
{
  const MooseObject * myself_again = dynamic_cast<const MooseObject *>(&me);
  stream << "\n\n"
         << "MutableCoefficientsInterface: " << (myself_again ? myself_again->name() : "Unknown")
         << "\n"
         << "      Number of Coefficients: " << me.getSize() << "\n";

  for (std::size_t i = 0; i < me.getSize(); ++i)
    stream << std::setw(4) << i << ": " << std::setw(12) << me[i] << ((i % 6 == 5) ? "\n" : "    ");

  stream << "\n\n" << std::flush;

  return stream;
}
