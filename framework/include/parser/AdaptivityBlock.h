#ifndef ADAPTIVITYBLOCK_H
#define ADAPTIVITYBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class AdaptivityBlock;

template<>
InputParameters validParams<AdaptivityBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<unsigned int>("steps", 0, "The number of adaptivity steps to perform at any one time for steady state", false);
  params.addParam<unsigned int>("initial_adaptivity", 0, "The number of adaptivity steps to perform using the initial conditions", false);
  params.addParam<Real> ("refine_fraction", 0.0, "The fraction of elements or error to refine. Should be between 0 and 1.", false);
  params.addParam<Real> ("coarsen_fraction", 0.0, "The fraction of elements or error to coarsen. Should be between 0 and 1.", false);
  params.addParam<unsigned int> ("max_h_level", 0, "Maximum number of times a single element can be refined. If 0 then infinite.", false);
  params.addParam<std::vector<std::string> > ("weight_names", "List of names of variables that will be associated with weight_values", false);
  params.addParam<std::vector<Real> > ("weight_values", "List of values between 0 and 1 to weight the associated weight_names error by", false);
  params.addParam<std::string> ("error_estimator", "KellyErrorEstimator", "The class name of the error estimator you want to use.", false);
  return params;
}

class AdaptivityBlock: public ParserBlock
{
public:
  AdaptivityBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

  

#endif //ADAPTIVITYBLOCK_H
