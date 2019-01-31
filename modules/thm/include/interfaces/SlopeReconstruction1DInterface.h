#ifndef SLOPERECONSTRUCTION1DINTERFACE_H
#define SLOPERECONSTRUCTION1DINTERFACE_H

#include "MooseTypes.h"
#include "InputParameters.h"
#include "Enums.h"

class SlopeReconstruction1DInterface;
class MooseObject;

template <>
InputParameters validParams<SlopeReconstruction1DInterface>();

/**
 * Interface class for 1-D slope reconstruction
 *
 * This class provides interfaces for generating slopes for an arbitrary set
 * of variables. A number of reconstruction options are provided, including a
 * number of TVD slope limiters.
 */
class SlopeReconstruction1DInterface
{
public:
  SlopeReconstruction1DInterface(const MooseObject * moose_object);

  /// Slope reconstruction type
  enum ESlopeReconstructionType
  {
    None,    ///< No reconstruction; Godunov scheme
    Full,    ///< Full reconstruction; no limitation
    Minmod,  ///< Minmod slope limiter
    MC,      ///< Monotonized Central-Difference slope limiter
    Superbee ///< Superbee slope limiter
  };
  /// Map of slope reconstruction type string to enum
  static const std::map<std::string, ESlopeReconstructionType> _slope_reconstruction_type_to_enum;

  /**
   * Gets a MooseEnum for slope reconstruction type
   *
   * @param[in] name   default value
   * @returns MooseEnum for slope reconstruction type
   */
  static MooseEnum getSlopeReconstructionMooseEnum(const std::string & name = "");

protected:
  /**
   * Gets limited slopes for the primitive variables in the 1-D direction
   *
   * @param[in] elem   Element for which to get slopes
   *
   * @returns Vector of slopes for the element in the 1-D direction
   */
  std::vector<Real> getElementSlopes(const Elem * elem) const;

  /**
   * Computes the cell-average primitive variable values for an element
   *
   * @param[in] elem   Element for which to get values
   *
   * @returns Vector of values on element
   */
  virtual std::vector<Real> computeElementPrimitiveVariables(const Elem * elem) const = 0;

  /// Slope reconstruction scheme
  const ESlopeReconstructionType _scheme;

  /// Number of sides
  static const unsigned int _n_side;
  /// Number of elemental values in stencil for computing slopes
  static const unsigned int _n_sten;
};

namespace THM
{
template <>
SlopeReconstruction1DInterface::ESlopeReconstructionType
stringToEnum<SlopeReconstruction1DInterface::ESlopeReconstructionType>(const std::string & s);
}

#endif
