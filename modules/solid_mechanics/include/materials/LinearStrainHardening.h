#ifndef LINEARSTRAINHARDENING_H
#define LINEARSTRAINHARDENING_H

#include "SolidModel.h"

class LinearStrainHardening : public SolidModel
{
public:
  LinearStrainHardening( const std::string & name,
                         InputParameters parameters );
  virtual ~LinearStrainHardening() {}

};

template<>
InputParameters validParams<LinearStrainHardening>();

#endif //LINEARSTRAINHARDENING_H
