#include "MooseInit.h"
#include "Parser.h"
#include "Executioner.h"
#include "MooseTest.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Moose Test");

int
main(int argc, char *argv[])
{
  MooseInit init(argc, argv);
  MooseTest::registerObjects();

  Parser p;

  std::string input_filename = "";
  if (Moose::command_line->search("-i"))
    input_filename = Moose::command_line->next(input_filename);
  else
    p.printUsage();

// '1' to use Actions '0' to use old Parsing System
#if 1
  p.parse_new(input_filename);
  p.execute_new();
#else
  p.parse(input_filename);
  p.execute();
#endif

  Executioner *e = p.getExecutioner();
  e->execute();
  delete e;
  

  return 0;
}
