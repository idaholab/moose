#ifndef SLOPERECONSTRUCTION1DINTERFACE_H
#define SLOPERECONSTRUCTION1DINTERFACE_H

#include "FlowModel.h"

class SlopeReconstruction1DInterface;

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
  const FlowModel::ESlopeReconstructionType _scheme;

  /// Number of sides
  static const unsigned int _n_side;
  /// Number of elemental values in stencil for computing slopes
  static const unsigned int _n_sten;
};

#endif
