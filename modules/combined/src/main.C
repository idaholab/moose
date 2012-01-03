/**
 * Elk Application
 */

#include "Elk.h"

//Moose Includes
#include "MooseInit.h"
#include "Executioner.h"
#include "Moose.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"
#include "ElkSyntax.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Elk");

 // Begin the main program.
int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  Elk::registerObjects();
  
  // Associate Parser Syntax
  Moose::associateSyntax();
  Elk::associateSyntax();
  Parser p(Moose::syntax);
  p.setCheckUnusedFlag( true );

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();

  p.parse(input_filename);
  p.execute();

  Moose::executioner->execute();

  return 0;
}
