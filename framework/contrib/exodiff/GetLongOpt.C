/* S Manoharan. Advanced Computer Research Institute. Lyon. France */
#include <GetLongOpt.h>
#include <string.h>
#include <sstream>

GetLongOption::GetLongOption(const char optmark)
  : table(NULL), ustring(NULL), pname(NULL), last(NULL),
    enroll_done(0), optmarker(optmark)
{
   ustring = "[valid options and arguments]";
}

GetLongOption::~GetLongOption()
{
   Cell *t = table;

   while ( t ) {
      Cell *tmp = t;
      t = t->next;
      delete tmp;
   }
}

char *
GetLongOption::basename(char * const pathname)
{
   char *s;

   s = strrchr(pathname, '/');
   if ( s == 0 ) s = pathname;
   else ++s;

   return s;
}

int
GetLongOption::enroll(const char * const opt, const OptType t,
		   const char * const desc,
		   const char * const val, const char * const optval)
{
   if ( enroll_done ) return 0;

   Cell *c = new Cell;
   c->option = opt;
   c->type = t;
   c->description = desc ? desc : "no description available";
   c->value = val;
   c->opt_value = optval;
   c->next = 0;

   if ( last == 0 ) {
      table = last = c;
   }
   else {
      last->next = c;
      last = c;
   }

   return 1;
}

const char *
GetLongOption::retrieve(const char * const opt) const
{
   Cell *t;
   for ( t = table; t != 0; t = t->next ) {
      if ( strcmp(opt, t->option) == 0 )
	 return t->value;
   }
   std::cerr << "GetLongOption::retrieve - unenrolled option ";
   std::cerr << optmarker << opt << "\n";
   return 0;
}

int
GetLongOption::parse(int argc, char * const *argv)
{
   int my_optind = 1;

   pname = basename(*argv);
   enroll_done = 1;
   if ( argc-- <= 1 ) return my_optind;

   while ( argc >= 1 ) {
      char *token = *++argv; --argc;

      // '--' signifies end of options if followed by space
      if ( token[0] != optmarker || (token[1] == optmarker && strlen(token) == 2))
	 break;	/* end of options */

      ++my_optind;
      char *tmptoken = ++token;
      if (token[0] == optmarker) // Handle a double '--'
	tmptoken = ++token;

      while ( *tmptoken && *tmptoken != '=' )
	 ++tmptoken;
      /* (tmptoken - token) is now equal to the command line option
	 length. */

      Cell *t;
      enum { NoMatch, ExactMatch, PartialMatch, MultipleMatch}
      matchStatus = NoMatch;

      std::ostringstream errmsg;
      Cell *pc = 0;	// pointer to the partially-matched cell
      for ( t = table; t != 0; t = t->next ) {
	 if ( strncmp(t->option, token, (tmptoken - token)) == 0 ) {
	    if ( (int)strlen(t->option) == (tmptoken - token) ) {
	       /* an exact match found */
	       int stat = setcell(t, tmptoken, *(argv+1), pname);
	       if ( stat == -1 ) return -1;
	       else if ( stat == 1 ) {
		  ++argv; --argc; ++my_optind;
	       }
	       matchStatus = ExactMatch;
	       break;
	    }
	    else {
	       /* partial match found */
	       if (pc == 0) {
		 matchStatus = PartialMatch;
		 pc = t;
	       } else {
		 // Multiple partial matches...Print warning
		 if (matchStatus == PartialMatch) {
		   // First time, print the message header and the first
		   // matched duplicate...
		   errmsg << "ERROR: " << pname
			  << ": Multiple matches found for option '"
			  << optmarker << strtok(token,"= ") << "'.\n";
		   errmsg << "\t" << optmarker << pc->option << ": "
			  << pc->description << "\n";
		 }
		 errmsg << "\t" << optmarker << t->option  << ": "
			<<  t->description << "\n";
		 matchStatus = MultipleMatch;
	       }
	    }
	 } /* end if */
      } /* end for */

      if ( matchStatus == PartialMatch ) {
	 int stat = setcell(pc, tmptoken, *(argv+1), pname);
	 if ( stat == -1 ) return -1;
	 else if ( stat == 1 ) {
	    ++argv; --argc; ++my_optind;
	 }
      }
      else if ( matchStatus == NoMatch ) {
	 std::cerr << pname << ": unrecognized option ";
	 std::cerr << optmarker << strtok(token,"= ") << "\n";
	 return -1;		/* no match */
      }
      else if ( matchStatus == MultipleMatch ) {
	std::cerr << errmsg.str();
	return -1;		/* no match */
      }

   } /* end while */

   return my_optind;
}

int
GetLongOption::parse(char * const str, char * const p)
{
   enroll_done = 1;
   char *token = strtok(str, " \t");
   const char *name = p ? p : "GetLongOption";

   while ( token ) {
     if ( token[0] != optmarker || (token[1] == optmarker && strlen(token) == 2)) {
	 std::cerr << name << ": nonoptions not allowed\n";
	 return -1;	/* end of options */
      }

      char *ladtoken = 0;	/* lookahead token */
      char *tmptoken = ++token;
      while ( *tmptoken && *tmptoken != '=' )
	 ++tmptoken;
      /* (tmptoken - token) is now equal to the command line option
	 length. */

      Cell *t;
      enum { NoMatch, ExactMatch, PartialMatch } matchStatus = NoMatch;
      Cell *pc =0;	// pointer to the partially-matched cell
      for ( t = table; t != 0; t = t->next ) {
	 if ( strncmp(t->option, token, (tmptoken - token)) == 0 ) {
	    if ( (int)strlen(t->option) == (tmptoken - token) ) {
	       /* an exact match found */
	       ladtoken = strtok(0, " \t");
	       int stat = setcell(t, tmptoken, ladtoken, name);
	       if ( stat == -1 ) return -1;
	       else if ( stat == 1 ) {
		  ladtoken = 0;
	       }
	       matchStatus = ExactMatch;
	       break;
	    }
	    else {
	       /* partial match found */
	       matchStatus = PartialMatch;
	       pc = t;
	    }
	 } /* end if */
      } /* end for */

      if ( matchStatus == PartialMatch ) {
	 ladtoken = strtok(0, " \t");
	 int stat = setcell(pc, tmptoken, ladtoken, name);
	 if ( stat == -1 ) return -1;
	 else if ( stat == 1 ) {
	    ladtoken = 0;
	 }
      }
      else if ( matchStatus == NoMatch ) {
	 std::cerr << name << ": unrecognized option ";
	 std::cerr << optmarker << strtok(token,"= ") << "\n";
	 return -1;		/* no match */
      }

      token = ladtoken ? ladtoken : strtok(0, " \t");
   } /* end while */

   return 1;
}

/* ----------------------------------------------------------------
GetLongOption::setcell returns
   -1	if there was an error
    0	if the nexttoken was not consumed
    1	if the nexttoken was consumed
------------------------------------------------------------------- */

int
GetLongOption::setcell(Cell *c, char *valtoken, char *nexttoken, const char *name)
{
   if ( c == 0 ) return -1;

   switch ( c->type ) {
   case GetLongOption::NoValue :
      if ( *valtoken == '=' ) {
	 std::cerr << name << ": unsolicited value for flag ";
	 std::cerr << optmarker << c->option << "\n";
	 return -1;	/* unsolicited value specification */
      }
      // Set to a non-zero value.  Used to be "(char*) ~0", but that
      // gives out-of-range warnings on some systems...
      c->value = (char *) 1;
      return 0;
   case GetLongOption::OptionalValue :
      if ( *valtoken == '=' ) {
	 c->value = ++valtoken;
	 return 0;
      }
      else {
	 if ( nexttoken != 0 && nexttoken[0] != optmarker ) {
	    c->value = nexttoken;
	    return 1;
	 }
	 else {
	   c->value = c->opt_value;
	   return 0;
	 }
      }
   case GetLongOption::MandatoryValue :
      if ( *valtoken == '=' ) {
	 c->value = ++valtoken;
	 return 0;
      }
      else {
	// KLUGE for exodiff "-steps -1" parsing.  need to do a better way...
	if ( nexttoken != 0 && (nexttoken[0] != optmarker || (nexttoken[0] == optmarker && nexttoken[1] == '1') )) {
	    c->value = nexttoken;
	    return 1;
	 }
	 else {
	    std::cerr << name << ": mandatory value for ";
	    std::cerr << optmarker << c->option << " not specified\n";
	    return -1;	/* mandatory value not specified */
	 }
      }
   default :
      break;
   }
   return -1;
}

void
GetLongOption::usage(std::ostream &outfile) const
{
   Cell *t;

   outfile << "\nusage: " << pname << " " << ustring << "\n";
   for ( t = table; t != 0; t = t->next ) {
      outfile << "\t" << optmarker << t->option;
      if ( t->type == GetLongOption::MandatoryValue )
	 outfile << " <$val>";
      else if ( t->type == GetLongOption::OptionalValue )
	 outfile << " [$val]";
      outfile << " (" << t->description << ")\n";
   }
   outfile.flush();
}
