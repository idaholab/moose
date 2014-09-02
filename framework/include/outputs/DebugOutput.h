/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DEBUGOUTPUT_H
#define DEBUGOUTPUT_H

// MOOSE includes
#include "PetscOutput.h"
#include "FEProblem.h"

// Forward declerations
class DebugOutput;

template<>
InputParameters validParams<DebugOutput>();


/**
 * A structure for storing data related to top residuals
 *  @see DebugOutput::printTopResiduals()
 */
struct DebugOutputTopResidualData
{
  unsigned int _var;
  unsigned int _nd;
  Real _residual;

  DebugOutputTopResidualData() { _var = 0; _nd = 0; _residual = 0.; }

  DebugOutputTopResidualData(unsigned int var, unsigned int nd, Real residual)
  {
    _var = var;
    _nd = nd;
    _residual = residual;
  }
};

/**
 * A class for producing various debug related outputs
 *
 * This class may be used from inside the [Outputs] block or via the [Debug] block (preferred)
 */
class DebugOutput : public PetscOutput
{
public:

  /**
   * Class constructor
   * @param name Output object name
   * @param parameters Object input parameters
   */
  DebugOutput(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~DebugOutput();

protected:

  /**
   * Perform the debugging output
   */
  virtual void output();

  /**
   * Prints material property information in a format similar to Moose system information
   */
  void printMaterialMap() const;

  /**
   * Builds a output streams for the properties in each material object
   * @param output The output stream to populate
   * @param materials Vector of pointers to the Material objects of interest
   */
  void printMaterialProperties(std::stringstream & output, const std::vector<Material * > & materials) const;

  /**
   * Prints the n top residuals for the variables in the system
   * @param residual A reference to the residual vector
   * @param n The number of residuals to print
   */
  void printTopResiduals(const NumericVector<Number> & residual, unsigned int n);

  /**
   * Method for sorting the residuals data from DebugOutputTopResidualData structs
   * @see printTopResiduals
   */
  static bool sortTopResidualData(DebugOutputTopResidualData i, DebugOutputTopResidualData j) { return (fabs(i._residual) > fabs(j._residual)); }

  //@{
  /**
   * Individual component output is not supported for DebugOutput
   */
  std::string filename();
  virtual void outputNodalVariables();
  virtual void outputElementalVariables();
  virtual void outputPostprocessors();
  virtual void outputVectorPostprocessors();
  virtual void outputScalarVariables();
  //@}

  /// Number of residuals to display
  unsigned int _show_top_residuals;

  /// Flag for outputting variable norms
  bool _show_var_residual_norms;

  /// Reference to libMesh system
  TransientNonlinearImplicitSystem & _sys;

};

#endif //DEBUGOUTPUT_H
