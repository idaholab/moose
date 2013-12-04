//  Base class for effective saturation as a function of capillary pressure
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
  virtual Real seff(Real pc) const;
  virtual Real dseff(Real pc) const;
  virtual Real d2seff(Real pc) const;

};

#endif // RICHARDSSEFF_H
