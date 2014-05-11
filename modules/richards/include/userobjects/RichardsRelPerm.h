/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSRELPERM_H
#define RICHARDSRELPERM_H

#include "GeneralUserObject.h"

class RichardsRelPerm;


template<>
InputParameters validParams<RichardsRelPerm>();

/**
 * Base class for Richards relative permeability classes
 * that provide relative permeability as a function of effective saturation
 */
class RichardsRelPerm : public GeneralUserObject
{
 public:
  RichardsRelPerm(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  /**
   * relative permeability as a function of effective saturation
   * This must be over-ridden in your derived class to provide actual
   * values of relative permeability
   * @param seff effective saturation
   */
  virtual Real relperm(Real seff) const = 0;

  /**
   * derivative of relative permeability wrt effective saturation
   * This must be over-ridden in your derived class to provide actual
   * values of derivative of relative permeability
   * @param seff effective saturation
   */
  virtual Real drelperm(Real seff) const = 0;

  /**
   * second derivative of relative permeability wrt effective saturation
   * This must be over-ridden in your derived class to provide actual
   * values of second derivative of relative permeability
   * @param seff effective saturation
   */
  virtual Real d2relperm(Real seff) const = 0;

};

#endif // RICHARDSRELPERM_H
