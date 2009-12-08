#ifndef OUTPUTBLOCK_H
#define OUTPUTBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class OutputBlock;

template<>
InputParameters validParams<OutputBlock>()
{
  InputParameters params = validParams<ParserBlock>();

  params.addParam<std::string>("file_base", "out", "The desired solution output name without an extension");
  params.addParam<int>("interval", 1, "The iterval at which timesteps are output to the solution file");
  params.addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)");
  params.addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)");
  params.addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)");
  params.addParam<bool>("print_out_info", false, "Specifies that you would like to see more verbose output information on STDOUT");
  params.addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file");
  return params;
}

class OutputBlock: public ParserBlock
{
public:
  OutputBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

#endif //OUTPUTBLOCK_H
