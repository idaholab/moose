/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Base class for effective saturation as a function of pressure(s)
//
#ifndef RICHARDSSEFF_H
#define RICHARDSSEFF_H

#include "GeneralUserObject.h"

class RichardsSeff;


template<>
InputParameters validParams<RichardsSeff>();

class RichardsSeff : public GeneralUserObject
{
 public:
  RichardsSeff(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  // These functions must be over-ridden in the derived class
  // to provide the actual values of seff and its derivatives
  virtual Real seff(std::vector<VariableValue *> p, unsigned int qp) const = 0;
  virtual std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const = 0;
  virtual std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const = 0;

};

#endif // RICHARDSSEFF_H
