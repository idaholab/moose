#ifndef ADDPOSTPROCESSORACTION_H
#define ADDPOSTPROCESSORACTION_H

#include "MooseObjectAction.h"

class AddPostprocessorAction: public MooseObjectAction
{
public:
  AddPostprocessorAction(const std::string & name, InputParameters params);

  virtual void act();

protected:
  Moose::PostprocessorType _pps_type;
};

template<>
InputParameters validParams<AddPostprocessorAction>();  

#endif //ADDPOSTPROCESSORACTION_H
