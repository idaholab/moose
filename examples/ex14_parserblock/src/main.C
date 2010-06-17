/**
 * Example 14: Custom Parser Block
 * This example runs our favorite Convection Diffusion problems but uses
 * a custom parser block to add the kernels all together with one new
 * block in our input file instead of listing everything explicitly
 */

//Moose Includes
#include "Parser.h"
#include "Executioner.h"
#include "MooseSystem.h"
#include "KernelFactory.h"
#include "ParserBlockFactory.h"    // <- Need to include the ParserBlockFactory

// Example 14 Includes
#include "Convection.h"
#include "ConvectionDiffusionBlock.h"

// C++ include files
#include <iostream>

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 14: Custom Parser Block");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  MooseSystem moose_system;
  
  Moose::registerObjects();

  KernelFactory::instance()->registerKernel<Convection>("Convection");

  /**
   * Registering a Parser Block is a little different than registering the other MOOSE
   * objects.  The name of what you register should match the fully qualified block
   * name where this block is expected in the input file.  The wildcard character can
   * also be used provided it replaces an entire "directory" (between parens) if
   * your block should be matched for multiple cases.  One final thing to note is that you
   * don't specify a leading slash ever in your registration names
   */
  ParserBlockFactory::instance()->registerParserBlock<ConvectionDiffusionBlock>("Kernels/ConvectionDiffusion");

  Parser p(moose_system);
  
  std::string input_filename = "";
  if ( Moose::command_line->search("-i") )
    input_filename = Moose::command_line->next(input_filename);
  else
    mooseError("Must specify an input file using -i");
  
  p.parse(input_filename);
  p.execute();

  if(!Moose::executioner)
    mooseError("Executioner not supplied!");
  
  Moose::executioner->setup();
  Moose::executioner->execute();
}
