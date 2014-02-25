#  -*- python -*-
#  GetPot Version 1.0                                        Sept/13/2002
#
#  WEBSITE: http://getpot.sourceforge.net
#
#  This library is  free software; you can redistribute  it and/or modify
#  it  under  the terms  of  the GNU  Lesser  General  Public License  as
#  published by the  Free Software Foundation; either version  2.1 of the
#  License, or (at your option) any later version.
#
#  This library  is distributed in the  hope that it will  be useful, but
#  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
#  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
#  Lesser General Public License for more details.
#
#  You  should have  received a  copy of  the GNU  Lesser  General Public
#  License along  with this library; if  not, write to  the Free Software
#  Foundation, Inc.,  59 Temple Place,  Suite 330, Boston,  MA 02111-1307
#  USA
#
#  (C) 2001, 2002 Frank R. Schaefer
#==========================================================================
#<<:BEGIN-FRAME FRAMEWORK/code.python>>
import string
import os
import copy

class GetPot_variable:
    def __init__(self, name, str_value):
        self.name = name
        self.take(str_value)

    def take(self, str_value):
        self.value    = string.split(str_value)
        self.original = str_value


class GetPot:
#<<:BEGIN-FUNCTIONS>>

#<<BEGIN init/code.python>>
    def __init__(self, Argv=None, Filename=None):
        # in case a search for a specific argument failed,
        # it effects the next functions block.
        self.search_failed_f = 0

        # indeces of arguments that do not start with minus
        self.idx_nominus = []

        # vector of identified variables
        # (arguments of the form "variable=value")
        self.variables = [ ]

        self.section_list = []

        self.comments = {}

        # cursor oriented functions (nect(), follow(), etc.):
        # pointer to actual position to be parsed.
        self.cursor         = 0
        self.nominus_cursor = -1
        self.search_loop_f  = 1
        self.prefix         = ""
        # set up the internal database

        if Filename != None:
            Argv = [ Filename ]
            parsed_argv = self.__read_in_file(Filename)
            try:    Argv.extend(parsed_argv)
            except: pass

        self.argv = self.__parse_argument_vector(Argv)

#<<END init/code.python BEGIN parse_argv/code.python>>
    def __parse_argument_vector(self, argv_):

        self.section = ''
        section_stack = []

        argv = []

        for i in range(len(argv_)):
            arg = argv_[i]

            if len(arg) == 0: continue
            elif i == 0:      argv.append(arg); continue

            # [section] ?
            if len(arg) > 1 and arg[0] == '[' and arg[-1] == ']':
                name = self.DBE_expand_string(arg[1:-1])
                self.section = self.__process_section_label(name, section_stack)
                if self.section not in self.section_list:
                    self.section_list.append(self.section)
                argv.append(arg)
            else:
                arg = self.section + self.DBE_expand_string(arg[:])
                argv.append(arg)

            # no-minus argument ?
            if arg[0] != '-': self.idx_nominus.append(i)

            # assignment ?
            for k in range(len(arg)-1):
                if arg[k] == '=':
                    v = self.__find_variable(arg[0:k])
                    if v == None:
                        self.variables.append(GetPot_variable(arg[0:k], arg[k+1:]))
                    else:
                        v.take(arg[k+1:])
        return argv

#<<END parse_argv/code.python BEGIN parse_file/code.python>>
    # (*) file parsing
    def __read_in_file(self, Filename):
        """Parses a file and returns a vector of arguments."""
        try:    fh = open(Filename, "rb")
        except: return None

        brute_tokens = []
        token = 0
        while token != '':
            self.current_comment = ''
            self.__skip_whitespace(fh)
            token = self.__get_next_token(fh)
            brute_tokens.append(token)
            if self.current_comment != '':
                self.comments[token] = self.current_comment

        # -- reduce expressions of token1'='token2 to a single
        #    string 'token1=token2'
        # -- copy everything into 'argv'
        # -- arguments preceded by something like '[' name ']' (section)
        #    produce a second copy of each argument with a prefix '[name]argument'
        i1 = 0; i2 = 1; i3 = 2;

        argv = []
        # loop over brute tokens to create argv vector
        while i1 < len(brute_tokens):
            SRef = brute_tokens[i1];

            # concatinate 'variable' '=' 'value' to 'variable=value'
            if i2 < len(brute_tokens) and brute_tokens[i2] == '=':
                if i3 >= len(brute_tokens):
                    argv.append(brute_tokens[i1] + brute_tokens[i2])
                else:
                    argv.append(brute_tokens[i1] + brute_tokens[i2] + brute_tokens[i3])
                i1 = i3 + 1; i2 = i3 + 2; i3 = i3 + 3;
                continue
            else:
                argv.append(SRef)
                i1 = i2; i2 = i3; i3 += 1;

        return argv

    def __skip_whitespace(self, FH):
        """Skips whitespaces: space, tabulator, newline and #-comments."""
        tmp = ' '
        while 1+1==2:
            while tmp == ' ' or tmp == '\t' or tmp == '\n':
                tmp = FH.read(1)
                if tmp == '': return     # end of file ?

            # found a non whitespace
            if tmp != '#':
                # put the last read letter back
                FH.seek(-1,1) # (seek -1 backwards from current position (code=1))
                return

            # '#' - comment => skip until end of line
            while tmp != '\n':
                tmp = FH.read(1)
                self.current_comment += tmp
                if tmp == '': return # end of file ?

    def __get_next_token(self, FH):
        """Reads next chunk of characters that are not separated by
        whitespace. Quotes and ${ ... }, however, allow to embrace whitespaces."""
        token = ''; tmp = 0; last_letter = 0
        while 1+1 == 2:
            last_letter = tmp; tmp = FH.read(1);
            if tmp == '#': #If we run into a comment symbol keep reading until the end of the line
                while tmp != '\n':
                    tmp = FH.read(1)
                    self.current_comment += tmp
                    if tmp == '': return token # end of file ?
                return token

            if tmp == '' or \
               ((tmp == ' ' or tmp == '\t' or tmp == '\n') and last_letter != '\\'):
                return token
            elif tmp == '\'' and not last_letter == '\\':
                # QUOTES: un-backslashed quotes => it's a string
                token += "'" + self.__get_string(FH) + "'"
                continue
            elif tmp == "{" and last_letter == '$':
                token += '{' + self.__get_until_closing_bracket(FH)
                continue
            elif tmp == "$" and last_letter == '\\':
                token += tmp; tmp = 0 # so that last_letter will become = 0, not '$'
                continue
            elif tmp == '\\' and not last_letter == '\\':
                continue              # don't append un-backslashed backslashes

            token += tmp


    def __get_string(self, FH):
        """Reads characters until the next un-backslashed quote."""
        str = ''; tmp = 0
        while 1 + 1 == 2:
            last_letter = tmp; tmp = FH.read(1)
            if tmp == '': return str
            # un-backslashed quotes => it's the end of the string
            elif   tmp == '\'' and not last_letter == '\\':  return str
            elif tmp == '\\' and not last_letter == '\\':  continue # don't append

            str += tmp


    def __get_until_closing_bracket(self, FH):
        """Reads characters until the next un-backslashed '}'."""
        str = ''; tmp = 0
        brackets = 1
        while 1 + 1 == 2:
            last_letter = tmp; tmp = FH.read(1)
            if tmp == '':                           return str
            elif tmp == '{' and last_letter == '$': brackets += 1
            elif tmp == '}':
                brackets -= 1
                # un-backslashed brackets => it's the end of the string
                if brackets == 0: return str + '}'
            elif tmp == '\\' and not last_letter == '\\':
                continue  # do not append an unbackslashed backslash

            str += tmp



    def __process_section_label(self, label, section_stack):
        #  1) subsection of actual section ('./' prefix)
        if len(label) >= 2 and label[:2] == "./":
            label = label[2:]
            # a single [./] means 'the same section'
        #  2) subsection of parent section ('../' prefix)
        elif label[0:3] == "../":
            while label[0:3] == "../":
                if len(section_stack) != 0: section_stack.pop()
                label = label[3:]
        # 3) subsection of the root-section
        else:
            del section_stack[:]

        # 4) parse section name for slashes
        if label != "":
            i=0
            while i < len(label):
                if label[i] == '/':
                    section_stack.append(label[0:i])
                    if i+1 < len(label):
                        label = label[i+1:]
                        i = 0
                else:
                    i += 1
            section_stack.append(label)

        section = ""
        for s in section_stack:
            section += s + '/'

        return section

#<<END parse_file/code.python BEGIN convert_to_type/code.python>>
    def __convert_to_type(self, String, Default):
        """Converts a string into an object of the same type as 'Default'.
        Returns 'None' in case this is not possible."""
        if type(Default) == type(""):
            # character string
            return String
        elif type(Default) == type(0.):
            # float
            try:    return float(String)
            except: return Default
        elif type(Default) == type(0):
            # integer
            if  len(String) >= 2 and String[0:2] == "0x":  start_i = 2
            elif len(String) >=3 and String[0:3] == "-0x": start_i = 3
            else:
                # normal integer, not a hexadecimal
                try:    return int(String)
                except: return Default

            # a hexadecimal number
            number = 0;
            for c in String[start_i:len(String)]:
                c = int(c)
                if c >= int('0') and c <= int('9'):   digit = c - int('0')
                elif c >= int('a') and c <= int('f'): digit = c - int('a')
                elif c >= int('A') and c <= int('F'): digit = c - int('A')
                else:                       break
                number *= 16
                number += digit
            if start_i == 2: return number
            else:            return -number

#<<END convert_to_type/code.python BEGIN search/code.python>>
    def __get_remaining_string(self, String, Start):
        """Checks if 'String' begins with 'Start' and returns the remaining String.
        Returns None if String does not begin with Start."""
        if Start == "": return String
        if string.find(String, Start) == 0: return String[len(Start):]
        else:                               return None

    #     -- search for a certain option and set cursor to position
    def search(self, *Args):
        """Search for a command line argument and set cursor. Starts search
        from current cursor position. Only wraps arround the end, if 'loop'
        is enabled. Returns '0' if nothing was found."""

        if self.cursor >= len(self.argv)-1:
            self.cursor = len(self.argv)-1
        self.search_failed_f = 1
        old_cursor = self.cursor

        def check_match(i0, i1, Args, Argv=self.argv, Prefix=self.prefix, obj=self):
            """Checks if one of the arguments in Args matches an argument in sequence."""
            for i in range(i0, i1):
                for arg in Args:
                    if Prefix + arg == Argv[i]:
                        obj.cursor = i; obj.search_failed_f = 0
                        return 1
            return 0

        # first run: from cursor to end
        if check_match(self.cursor, len(self.argv), Args) == 1: return 1

        if self.search_loop_f == 0: return 0

        # second run: from 1 to old_cursor position
        # (note, that old_cursor can be at maximum = len(self.argv),
        #  the range function contains therefore only values until
        #  "len(self.argv) - 1")
        if check_match(1, old_cursor, Args) == 1: return 1

        return 0

    def disable_loop(self):
        self.search_loop_f = 0

    def enable_loop(self):
        self.search_loop_f = 1

    #     -- reset cursor to initial position
    def reset_cursor(self):
        self.search_failed_f = 0; self.cursor = 0

    def search_failed(self):
        return self.search_failed_f

    def init_multiple_occurrence(self):
        self.disable_loop(); self.reset_cursor()

    def set_prefix(self, Prefix):
        self.prefix = Prefix

#<<END search/code.python BEGIN get/code.python>>
    # (*) direct access to command line arguments through []-operator
    def __getitem__(self, Idx):
        """Returns a specific argument indexed by Idx or 'None' if this
        does not exist."""
        if Idx < 0 or Idx >= len(self.argv): return None
        return self.argv[Idx]

    def get(self, Idx, Default):
        """Looks if the type of argv[Idx] matches the type of the default argument.
        If it does not, the default argument is returned."""
        if self[Idx] == None: return Default
        return self.__convert_to_type(self[Idx], Default)

    def size(self):
        """Returns the size of the argument list."""
        return len(self.argv)


#<<END get/code.python BEGIN next/code.python>>
    #     -- get argument at cursor++
    def next(self, Default):
        """Tests if the following argument is of the same type as Default. If not
        Default is returned. Note, that if the following argument does not contain
        the 'prefix', the same way the Default argument is returned."""
        if self.search_failed_f == 1: return Default
        self.cursor += 1
        if self.cursor >= len(self.argv): self.cursor = len(self.argv)-1; return Default

        if self.prefix == "": return self.__convert_to_type(self.argv[self.cursor], Default)

        remain = self.__get_remaining_string(self.argv[self.cursor], self.prefix)
        if remain != None: return self.__convert_to_type(remain, Default)
        else:              return Default


#<<END next/code.python BEGIN follow/code.python>>
    #     -- search for option and get argument at cursor++
    def follow(self, Default, *Args):
        for arg in Args:
            if self.search(arg) == 1:
                return self.next(Default)
        return Default


#<<END follow/code.python BEGIN direct_follow/code.python>>
    def direct_follow(self, Default, Arg):
        remaining_string = self.__match_starting_string(Arg)

        if remaining_string == None:
            return Default
        self.cursor += 1
        if self.cursor >= len(self.argv): self.cursor = len(self.argv)
        return self.__convert_to_type(remaining_string, Default)

    # helper to find directly followed arguments
    def __match_starting_string(self, StartString):
        """Searches argument list for next occurrence of 'StartString', beginning
        from current cursor position. Returns string after StartString if found.
        Returns None if no argument contains the starting string."""
        old_cursor = self.cursor

        self.search_failed_f = 1
        # first run: from cursor to end
        if self.cursor < len(self.argv):
            for i in range(old_cursor, len(self.argv)):
                if string.find(self.argv[i], StartString) == 0:
                    self.cursor = i
                    self.search_failed_f = 0
                    return self.argv[i][len(StartString):]

        if self.search_loop_f == 0: return None

        # second run: from 1 to old_cursor position
        # (note, that old_cursor can be at maximum = len(self.argv),
        #  the range function contains therefore only values until
        #  "len(self.argv) - 1")
        for i in range(1, old_cursor):
            if string.find(self.argv[i], StartString) == 0:
                self.cursor = i
                self.search_failed_f = 0
                return self.argv[i][len(StartString):]
        return None

#<<END direct_follow/code.python BEGIN flags/code.python>>
    # (*) flags
    def  options_contain(self, FlagList):
        """Go through all arguments that start with a '-' and watch if they
        contain a flag in flaglist. In case a prefix is specified, the option
        must be preceeded with it, e.g. 'pack-options/-cvx'."""
        for arg in self.argv:
            if self.prefix != "": arg = self.__get_remaining_string(arg, self.prefix)
            if arg != None and len(arg) >= 2 and arg[0] == '-' and arg[1] != '-' \
               and self.__check_flags(arg, FlagList) == 1: return 1

        return 0

    def  argument_contains(self, Idx, FlagList):
        """Check if an argument that is associated with a certain index contains
        a certain flag. If a prefix is specified, the index indicates the number
        inside the list."""
        if Idx < 0 or Idx > len(self.argv): return 0

        if self.prefix == "":
            # search argument for any flag in flag list
            return self.__check_flags(self.argv[Idx], FlagList)

        # if a prefix is set, then the argument index is the index
        #   inside the 'namespace'
        # => only check list of arguments that start with prefix
        no_matches = 0
        for i in range(len(self.argv)):
            remain = self.__get_remaining_string(self.argv[i], self.prefix)
            if remain != None:
                no_matches += 1
                if no_matches == Idx:
                    return self.__check_flags(remain, FlagList)
        # no argument in this namespace
        return 0

    def __check_flags(self, Str, FlagList):
        """Does a given string 'Str' contain a flag in 'FlagList' ?"""
        for l in Str:
            for f in FlagList:
                if f == l:
                    return 1
        return 0


#<<END flags/code.python BEGIN nominus/code.python>>
    # (*) nominus arguments
    def reset_nominus_cursor(self):
        self.nominus_cursor = -1

    def nominus_vector(self):
        v_nm = []
        for i in self.idx_nominus:
            v_nm.append(self.argv[i])
        return v_nm

    def nominus_size(self):
        return len(self.idx_nominus)

    def next_nominus(self):
        if self.nominus_cursor >= len(self.idx_nominus)-1: return None
        self.nominus_cursor += 1
        return self.argv[self.idx_nominus[self.nominus_cursor]]


#<<END nominus/code.python BEGIN variables/code.python>>
    # (*) variables
    # helper to find arguments
    def get_variable_names(self):
        # return all variables for given prefix
        vars = []
        for v in self.variables:
            tmp = self.__get_remaining_string(v.name, self.prefix)
            if tmp != None: vars.append(tmp)
        return vars

    def get_section_names(self):
        return self.section_list

    # helper to find arguments
    def __find_variable(self, VarName):
        """Search for a variable in the array of variables."""
        v_name = self.prefix + VarName
        for v in self.variables:
            if v.name == v_name: return v
        return None

    #     -- scalar values and vectors
    def __call__(self, VarName, Default, Idx=-1):
        """Returns 'None' in case variable was not found or type did not match."""
        v = self.__find_variable(VarName)
        if v == None:
            return Default
        if Idx == -1:
            # variable has to be considered as a single value
            return self.__convert_to_type(v.original, Default)
        else:
            # variable interpreted as vector
            if Idx >= len(v.value):
                return Default
            return self.__convert_to_type(v.value[Idx], Default)

    def vector_variable_size(self):
        return variables.size()


#<<END variables/code.python BEGIN print/code.python>>
    def Print(self):
        print "argc = %i" % len(self.argv)
        for arg in self.argv:
            print "%s" % arg

    # (*) dollar bracket expressions (DBEs) ------------------------------------
    #
    #     1) Entry Function: DBE_expand_string()
    #        Takes a string such as
    #
    #          "${+ ${x} ${y}}   Subject-${& ${section} ${subsection}}:   ${title}"
    #
    #        calls DBE_expand() for each of the expressions
    #
    #           ${+ ${x} ${y}}
    #           ${& ${section} ${subsection}}
    #           ${Title}
    #
    #        and returns the string
    #
    #          "4711 Subject-1.01:   Mit den Clowns kamen die Schwaene"
    #
    #        assuming that
    #            x          = "4699"
    #            y          = "12"
    #            section    = "1."
    #            subsection = "01"
    #            title      = "Mit den Clowns kamen die Schwaene"
    #
    #      2) DBE_expand():
    #
    #           checks for the command, i.e. the 'sign' that follows '${'
    #           divides the argument list into sub-expressions using
    #           DBE_get_expr_list()
    #
    #           ${+ ${x} ${y}}                 -> "${x}"  "${y}"
    #           ${& ${section} ${subsection}}  -> "${section}" "${subsection}"
    #           ${Title}                       -> Nothing, variable expansion
    #
    #      3) DBE_expression_list():
    #
    #           builds a vector of unbracketed whitespace separated strings, i.e.
    #
    #           "  ${Number}.a ${: Das Marmorbild} AB-${& Author= ${Eichendorf}-1870}"
    #
    #           is split into a vector
    #
    #              [0] ${Number}.a
    #              [1] ${: Das Marmorbild}
    #              [2] ${& Author= ${Eichendorf}}
    #
    #           The each sub-expression is expanded using expand().
    #---------------------------------------------------------------------------
    def DBE_expand_string(self, String):
        """Parses for closing operators '${ }' and expands them letting
           white spaces and other letters as they are."""
        new_string = ""
        open_brackets = 0
        for i in range(len(String)):
            if i < len(String)-2 and String[i:i+2] == "${":
                if open_brackets == 0: first = i+2;
                open_brackets += 1;
            elif String[i] == "}" and open_brackets > 0:
                open_brackets -= 1
                if open_brackets == 0:
                    new_string += self.DBE_expand(String[first:i])
            elif open_brackets == 0:
                new_string += String[i]

        return new_string


    def DBE_get_expr_list(self, String, ExpectedNumber):
        """Separates expressions by non-bracketed whitespaces, expands them
        and puts them into a list."""
        i = 0
        # (1) eat initial whitespaces
        for letter in String:
            if letter != " " and letter != "\t" and letter != "\n":
                break
            i += 1

        expr_list = []
        open_brackets = 0
        start_idx = []
        start_new_string = i
        L = len(String)
        # (2) search for ${ } expressions ...
        while i < L:
            letter = String[i]
            # whitespace -> end of expression
            if (letter == " " or letter == "\t" or letter == "\n") \
               and open_brackets == 0:
                expr_list.append(String[start_new_string:i])
                for i in range(i+1, L):
                    letter = String[i]
                    if letter != " " and letter != "\t" and letter != "\n":
                        start_new_string = i
                        break
                else:
                    # end of expression list
                    if len(expr_list) < ExpectedNumber:
                        expr_list.extend(["<< ${ }: missing arguments>>"] * (ExpectedNumber - len(expr_list)))
                    return expr_list

            # dollar-bracket expression
            if len(String) >= i+2 and String[i:i+2] == "${":
                open_brackets += 1
                start_idx.append(i+2)
            elif letter == "}" and open_brackets > 0:
                start = start_idx.pop()
                Replacement = self.DBE_expand(String[start:i])
                if start-2 <= 0: String = Replacement + String[i+1:]
                else:            String = String[:start-2] + Replacement + String[i+1:]
                L = len(String)
                i = start + len(Replacement) - 3
                open_brackets -= 1
            i += 1

        expr_list.append(String[start_new_string:i])
        if len(expr_list) < ExpectedNumber:
            expr_list.extend(["<< ${ }: missing arguments>>"] * (ExpectedNumber - len(expr_list)))

        return expr_list


    def DBE_get_variable(self, VarName):
        SECURE_Prefix = self.prefix

        for p in [self.section, ""]:
            self.prefix = p
            # (1) first search in currently active section
            # (2) search in root name space
            var = self.__find_variable(VarName)
            if type(var) != type(None):
                self.prefix = SECURE_Prefix
                return var

        self.prefix = SECURE_Prefix
        return "<<${ } variable '%s' undefined>>" % VarName


    def DBE_expand(self, Expr):
        # ${: } pure text
        if Expr[0] == ":":
            return Expr[1:]

        # ${& expr expr ... } text concatination
        elif Expr[0] == "&":
            A = self.DBE_get_expr_list(Expr[1:], 1)
            return reduce(lambda a,b: "%s%s" % (a, b), A)

        # ${<-> expr expr expr} text replacement
        elif len(Expr) >= 3 and Expr[0:3] == "<->":
            A = self.DBE_get_expr_list(Expr[3:], 3)
            #                     string  old   new
            return string.replace(A[0],   A[1], A[2])

        # ${+ ...}, ${- ...}, ${* ...}, ${/ ...} expressions
        elif Expr[0] == "+":
            A = self.DBE_get_expr_list(Expr[1:], 2)
            return "%e" % (reduce(lambda a, b:
                                  self.__convert_to_type(a, 0.) + self.__convert_to_type(b,0.),
                                  A))

        elif Expr[0] == "-":
            A = self.DBE_get_expr_list(Expr[1:], 2)
            return "%e" % reduce(lambda a, b:
                                 self.__convert_to_type(a, 0.) - self.__convert_to_type(b,0.),
                                 A)

        elif Expr[0] == "*":
            A = self.DBE_get_expr_list(Expr[1:], 2)
            return "%e" % reduce(lambda a, b:
                                 self.__convert_to_type(a, 0.) * self.__convert_to_type(b,0.),
                                 A)

        elif Expr[0] == "/":
            A = self.DBE_get_expr_list(Expr[1:], 2)
            Q = self.__convert_to_type(A[0], 0.)
            if Q == 0: return repr(0.)
            for q in A[1:]:
                q = self.__convert_to_type(q, 0.)
                if q == 0.0: return repr(0.)
                Q /= q
            return "%e" % Q

        # ${^ ... } power expressions
        elif Expr[0] == "^":
            A = self.DBE_get_expr_list(Expr[1:], 2)
            return "%e" % reduce(lambda a, b:
                                 self.__convert_to_type(a, 0.) ** self.__convert_to_type(b,0.),
                                 A)

        # ${==  } ${<=  } ${>= } comparisons (return the number of the first 'match'
        elif len(Expr) >= 2 and \
             (Expr[0:2] == "==" or Expr[0:2] == ">=" or Expr[0:2] == "<=" or \
             Expr[0:1] == ">"  or Expr[0:1] == "<"):
            # differentiate between two and one sign operators
            if Expr[1] == "=": OP = Expr[0:2]; A = self.DBE_get_expr_list(Expr[2:], 2)
            else:              OP = Expr[0];   A = self.DBE_get_expr_list(Expr[1:], 2)

            x_orig = A[0]
            x = self.__convert_to_type(x_orig, 1e37)
            i = 1

            for y_orig in A[1:]:
                y = self.__convert_to_type(y_orig, 1e37)
                # set the strings as reference if one wasn't a number
                if x == 1e37 or y == 1e37: xc = x_orig; y = y_orig;
                else:                      xc = x

                if   OP == "==" and xc == y: return repr(i)
                elif OP == ">=" and xc >= y: return repr(i)
                elif OP == "<=" and xc <= y: return repr(i)
                elif OP == ">"  and xc > y:  return repr(i)
                elif OP == "<"  and xc < y:  return repr(i)
                i += 1

            # nothing fulfills the condition => return 0
            return repr(0)

        # ${?? expr expr} select
        elif len(Expr) >=2 and Expr[0:2] == "??":
            A = self.DBE_get_expr_list(Expr[2:], 2)
            X = self.__convert_to_type(A[0], 1e37)
            # last element is always the default argument
            if X == 1e37 or X < 0 or X >= len(A) - 1:
                return A[-1]
            # round X to closest integer
            return A[int(X+0.5)]

        # ${? expr expr expr} if then else conditions
        elif Expr[0] == "?":
            A = self.DBE_get_expr_list(Expr[1:], 2)
            if self.__convert_to_type(A[0], 0.0) == 1.0: return A[1]
            elif len(A) > 2:                             return A[2]

        # ${! expr} maxro expansion
        elif Expr[0] == "!":
            Var = self.DBE_get_variable(Expr[1:])
            # error
            if type(Var) == type(""): return Var

            A = self.DBE_get_expr_list(Var.original, 2)
            return A[0]

        # ${@: } - string subscription
        elif len(Expr) >= 2 and Expr[0:2] == "@:":
            A = self.DBE_get_expr_list(Expr[2:], 2)
            X = self.__convert_to_type(A[1], 1e37)

            # last element is always the default argument
            if X == 1e37 or X < 0 or X >= len(A[0]) - 1:
                return "<<1st index out of range>>"

            if len(A) > 2:
                Y = self.__convert_to_type(A[2], 1e37)
                if Y != 1e37 and Y > 0 and Y <= len(A[0]) - 1 and Y > X:
                    return A[0][int(X+0.5):int(Y+1.5)]
                elif Y == -1:
                    return A[0][int(X+0.5):]
                return "<<2nd index out of range>>"
            else:
                return A[0][int(X+0.5)]

        # ${@ } - vector subscription
        elif Expr[0] == "@":
            A = self.DBE_get_expr_list(Expr[1:], 2)
            Var = self.DBE_get_variable(A[0])
            # error
            if type(Var) == type(""): return Var

            X = self.__convert_to_type(A[1], 1e37)

            # last element is always the default argument
            if X == 1e37 or X < 0 or X >= len(Var.value):
                return "<<1st index out of range>>"

            if len(A) > 2:
                Y = self.__convert_to_type(A[2], 1e37)
                if Y != 1e37 and Y > 0 and Y <= len(Var.value) and Y > X:
                    Vec = Var.value[int(X+0.5):int(Y+1.5)]
                elif Y == -1:
                    Vec = Var.value[int(X+0.5):]
                else:
                    return "<<2nd index out of range>>"
                return reduce(lambda a,b: "%s %s" % (a,b), Vec)
            else:
                return Var.value[int(X+0.5)]


        A = self.DBE_get_expr_list(copy.copy(Expr), 1)

        B = self.DBE_get_variable(A[0])

        if type(B) == type(""): return B
        else:                   return B.original


    # (*) unidentified flying objects
    def unidentified_arguments(self, *Knowns):
        ufos = []
        for it in self.argv[1:]:
            arg = self.__get_remaining_string(it, self.prefix)
            if arg not in Knowns: ufos.append(it)
        return ufos

    def unidentified_options(self, *Knowns):
        ufos = []
        for it in self.argv[1:]:
            arg = self.__get_remaining_string(it, self.prefix)
            if len(arg) < 2 or arg[0] != '-': continue
            if arg not in Knowns: ufos.append(it)
        return ufos

    def unidentified_flags(self, KnownFlags, ArgumentNumber=-1):
        ufos = ""
        if ArgumentNumber == -1:
            # (*) search through all options with one single '-'
            for it in self.argv[1:]:
                arg = self.__get_remaining_string(it, self.prefix)
                if len(arg) < 2:    continue
                elif arg[0] != '-': continue
                elif arg[1] == '-': continue

                for letter in arg[1:]:
                    if letter not in KnownFlags: ufos += letter;
        else:
            no_matches = 0
            for it in argv[1:]:
                Remain = self.__get_remaining_string(it, self.prefix)
                if Remain != "":
                    no_matches += 1
                    if no_matches == ArgumentNumber:
                        # -- the right argument number inside the section is found
                        # => check it for flags
                        for letter in Remain:
                            if letter not in KnownFlags: ufos += letter;
                        return ufos
        return ufos

    def unidentified_variables(self, *Knowns):
        ufos = []
        for it in self.variables:
            var_name = self.__get_remaining_string(it.name, self.prefix)
            if var_name not in Knowns: ufos.append(it.name)
        return ufos

    def unidentified_sections(self, *Knowns):
        ufos = []
        for it in self.section_list:
            sec_name = self.__get_remaining_string(it, self.prefix)
            if sec_name not in Knowns: ufos.append(it)
        return ufos

    def unidentified_nominuses(self, Knowns):
        ufos = []
        for it in self.argv[1:]:
            arg = self.__get_remaining_string(it, self.prefix)
            # only 'real nominuses'
            if   len(arg) < 1 or arg[0] == '-':    continue
            elif arg[0] == '[' and arg[-1] == ']': continue # section label
            elif '=' in arg:                       continue # variable definition

            if arg not in Knowns: ufos.append(it)
        return ufos

#<<END print/code.python >>
#<<:EPILOG>>
