// Copyright(C) 2008 Sandia Corporation.  Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// smart_assert.cpp
//
//////////////////////////////////////////////////////////////////////


#include "smart_assert.h"


#include <fstream>
#include <set>
#include <sstream>
#include <cstdlib>
#include <stdexcept>


void break_into_debugger() {
  std::cerr << "Not Implemented.\n";
}


namespace {
    // in case we're logging using the default logger...
    struct stream_holder {
        stream_holder() : out_( 0), owns_( false) {}
        ~stream_holder() {
            if ( owns_)
                delete out_;
            out_ = 0;
        }
        std::ostream * out_;
        bool owns_;
    };
    // information about the stream we write to, in case 
    // we're using the default logger
    stream_holder default_logger_info;

    // intitializes the SMART_ASSERT library
    struct assert_initializer {
        assert_initializer() {
            Private::init_assert();
        }
    } init;
} // anonymous namespace



namespace smart_assert {

    // returns a message corresponding to the type of level
    std::string get_typeof_level(int nLevel) {
        switch ( nLevel) {
        case lvl_warn: return "Warning";
        case lvl_debug: return "Assertion failed";
        case lvl_error: return "Assertion failed (Error)";
        case lvl_fatal: return "Assertion failed (FATAL)";
        default: {
            std::ostringstream out;
            out << "Assertion failed (level=" << nLevel << ")";
            return out.str();
                 }
        };
    }

    // helpers, for dumping the assertion context
    void dump_context_summary( const assert_context & context, std::ostream & out) {
        out << "\n" << get_typeof_level( context.get_level() ) 
            << " in " << context.get_context_file() << ":" << context.get_context_line() << '\n';
        if ( !context.get_level_msg().empty())
            // we have a user-friendly message
            out << context.get_level_msg();
        else
            out << "\nExpression: " << context.get_expr();
        out << std::endl;
    }

    void dump_context_detail( const assert_context & context, std::ostream & out) {
        out << "\n" << get_typeof_level( context.get_level() ) 
            << " in " << context.get_context_file() << ":" << context.get_context_line() << '\n';
        if ( !context.get_level_msg().empty())
            out << "User-friendly msg: '" << context.get_level_msg() << "'\n";
        out << "\nExpression: '" << context.get_expr() << "'\n";
        
        typedef assert_context::vals_array vals_array;
        const vals_array & aVals = context.get_vals_array();
        if ( !aVals.empty() ) {
            bool bFirstTime = true;
            vals_array::const_iterator first = aVals.begin(), last = aVals.end();
            while ( first != last) {
                if ( bFirstTime) {
                    out << "Values: ";
                    bFirstTime = false;
                }
                else {
                    out << "        ";
                }
                out << first->second << "='" << first->first << "'\n";
                ++first;
            }
        }
        out << std::endl;
    }

    ///////////////////////////////////////////////////////
    // logger

    void default_logger( const assert_context & context) {
        if ( default_logger_info.out_ == 0)
            return;
        dump_context_detail( context, *( default_logger_info.out_) );
    }

    ///////////////////////////////////////////////////////
    // handlers
    
    // warn : just dump summary to console
    void default_warn_handler( const assert_context & context) {
        dump_context_summary( context, std::cout);
    }


    // debug: ask user what to do
    void default_debug_handler( const assert_context & context) {
#if 1
        dump_context_detail( context, std::cerr);
	abort();
#else
        static bool ignore_all = false;
        if ( ignore_all)
            // ignore All asserts
            return;
        typedef std::pair< std::string, int> file_and_line;
        static std::set< file_and_line> ignorer;
        if ( ignorer.find( file_and_line( context.get_context_file(),
					  context.get_context_line())) != ignorer.end() )
            // this is Ignored Forever
            return;

        dump_context_summary( context, std::cerr );
        std::cerr << "\nPress (I)gnore/ Igore (F)orever/ Ignore (A)ll/ (D)ebug/ A(b)ort: ";
        std::cerr.flush();
        char ch = 0;

        bool bContinue = true;
        while ( bContinue && std::cin.get( ch)) {
            bContinue = false;
            switch ( ch) {
            case 'i': case 'I':
                // ignore
                break;

            case 'f': case 'F':
                // ignore forever
                ignorer.insert( file_and_line( context.get_context_file(),
					       context.get_context_line()));
                break;

            case 'a': case 'A':
                // ignore all
                ignore_all = true;
                break;

            case 'd': case 'D':
                // break
                break_into_debugger();
                break;

            case 'b': case 'B':
                abort();
                break;

            default:
                bContinue = true;
                break;
            }
        }
#endif
    }


    // error : throw a runtime exception
    void default_error_handler( const assert_context & context) {
        std::ostringstream out;
        dump_context_summary( context, out);
        throw std::runtime_error( out.str());
    }


    // fatal : dump error and abort
    void default_fatal_handler( const assert_context & context) {
        dump_context_detail( context, std::cerr);
        abort();
    }


} // namespace smart_assert


namespace Private {

    void init_assert() {
        Assert::set_log(                &::smart_assert::default_logger);
        Assert::set_handler( lvl_warn,  &::smart_assert::default_warn_handler);
        Assert::set_handler( lvl_debug, &::smart_assert::default_debug_handler);
        Assert::set_handler( lvl_error, &::smart_assert::default_error_handler);
        Assert::set_handler( lvl_fatal, &::smart_assert::default_fatal_handler);
    }

    // sets the default logger to write to this stream
    void set_default_log_stream( std::ostream & out) {
        default_logger_info.out_ = &out;
        default_logger_info.owns_ = false;
    }

    // sets the default logger to write to this file
    void set_default_log_name( const char * str) {
        default_logger_info.owns_ = false;
        default_logger_info.out_ = new std::ofstream( str);
        default_logger_info.owns_ = true;
    }

} // namespace Private
