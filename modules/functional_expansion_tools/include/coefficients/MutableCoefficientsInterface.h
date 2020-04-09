//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>

#include "InputParameters.h"
#include "MooseTypes.h"
#include "Restartable.h"

class ConsoleStream;

/**
 * This class is designed to provide a uniform interface for any class that uses an array of
 * coefficients for any of its operations. In particular, the MultiAppFXTransfer mechanism transfers
 * coefficients using this interface. Any derived class of MutableCoefficientsInterface can easily
 * be used in any MultiAppFXTransfer-based transfer.
 */
class MutableCoefficientsInterface : public Restartable
{
public:
  static InputParameters validParams();

  MutableCoefficientsInterface(const MooseObject * moose_object,
                               const InputParameters & parameters);

  // Coefficient access
  /**
   * Get the value of the coefficient at the corresponding index
   */
  Real operator[](std::size_t index) const;
  /**
   * Get a reference to the characteristics array
   */
  const std::vector<std::size_t> & getCharacteristics() const;
  /**
   * Get a read-only reference to the vector of coefficients
   */
  const std::vector<Real> & getCoefficients() const;
  /**
   * Get a writeable reference to the vector of coefficients
   */
  std::vector<Real> & getCoefficients();
  /**
   * Get a formatted string of the coefficients
   */
  std::string getCoefficientsTable() const;

  // Current state
  /**
   * Get the size, aka number of coefficients
   */
  std::size_t getSize() const;
  /**
   * Checks to see if another instance is compatible
   */
  bool isCompatibleWith(const MutableCoefficientsInterface & other) const;
  /**
   * Returns true if the size of the coefficient array is fixed and enforced
   */
  bool isSizeEnforced() const;
  /**
   * Toggle whether the size of the coefficient array can be changed
   */
  void enforceSize(bool enforce);

  // Mutable aspect
  /**
   * Import the coefficients from another instance
   */
  void importCoefficients(const MutableCoefficientsInterface & other);
  /**
   * Resize the array, using the value for fill if the new size is larger
   */
  void resize(std::size_t size, Real fill = 0.0, bool fill_out_to_size = true);
  /**
   * Sets the characteristics array
   */
  void setCharacteristics(const std::vector<std::size_t> & new_characteristics);
  /**
   * Set the coefficients using a copy operation
   */
  void setCoefficients(const std::vector<Real> & new_coefficients);
  /**
   * Set the coefficients using a move operation (only works with temp objects)
   */
  void setCoefficients(std::vector<Real> && dropin_coefficients);

  /**
   * Friend operator to easily print out the array of coefficients
   */
  friend std::ostream & operator<<(std::ostream & stream, const MutableCoefficientsInterface & me);

protected:
  /**
   * Called when the coefficients have been changed
   */
  virtual void coefficientsChanged(){};

  /// An array of integer characteristics that can be used to check compatibility
  std::vector<std::size_t> & _characteristics;

  /// The coefficient array
  std::vector<Real> & _coefficients;

  /// Boolean that locks or allows resizing of the coefficient array
  bool _enforce_size;

  /// Boolean to flag if the coefficients should be printed when set
  const bool _print_coefficients;

private:
  /// MooseObject instance of `this` to provide access to `_console`
  const ConsoleStream & _console;
};
