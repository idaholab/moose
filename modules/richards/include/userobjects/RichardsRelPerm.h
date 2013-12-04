//  Base class for relative permeability as a function of effective saturation
//
#ifndef RICHARDSRELPERM_H
#define RICHARDSRELPERM_H

#include "GeneralUserObject.h"

class RichardsRelPerm;


template<>
InputParameters validParams<RichardsRelPerm>();

class RichardsRelPerm : public GeneralUserObject
{
 public:
  RichardsRelPerm(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  // These functions must be over-ridden in the derived class
  // to provide the actual values of relative perm and its derivatives
  virtual Real relperm(Real seff) const;
  virtual Real drelperm(Real seff) const;
  virtual Real d2relperm(Real seff) const;

};

#endif // RICHARDSRELPERM_H
