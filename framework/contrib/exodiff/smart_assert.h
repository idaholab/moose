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
// smart_assert.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(SMART_ASSERT_H)
#define SMART_ASSERT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _MSC_VER > 1000

// note:
// moving this after pragma push will render it useless (VC6)
//
// identifier truncated to 255 chars in debug information
#pragma warning ( disable : 4786)

#pragma warning ( push )
// *this used in base-member initialization; it's ok
#pragma warning ( disable : 4355)
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <map>

enum {

    // default behavior - just loggs this assert
    // (a message is shown to the user to the console)
    lvl_warn = 100,

    // default behavior - asks the user what to do:
    // Ignore/ Retry/ etc.
    lvl_debug = 200,

    // default behavior - throws a smart_assert_error
    lvl_error = 300,

    // default behavior - dumps all assert context to console, 
    // and aborts
    lvl_fatal = 1000
};



/* 
    contains details about a failed assertion
*/
class assert_context {
    typedef std::string string;
public:
    assert_context() : level_( lvl_debug) {
    }

    // where the assertion failed: file & line
    void set_file_line( const char * file, int line) {
        file_ = file;
        line_ = line;
    }
    const string & get_context_file() const { return file_; }
    int get_context_line() const { return line_; }

    // get/ set expression
    void set_expr( const string & str) { expr_ = str; }
    const string & get_expr() const { return expr_; }

    typedef std::pair< string, string> val_and_str;
    typedef std::vector< val_and_str> vals_array;
    // return values array as a vector of pairs:
    // [Value, corresponding string]
    const vals_array & get_vals_array() const { return vals_; }
    // adds one value and its corresponding string
    void add_val( const string & val, const string & str) {
        vals_.push_back( val_and_str( val, str) );
    }

    // get/set level of assertion
    void set_level( int nLevel) { level_ = nLevel; }
    int get_level() const { return level_; }

    // get/set (user-friendly) message 
    void set_level_msg( const char * strMsg)  { 
        if ( strMsg)
            msg_ = strMsg; 
        else
            msg_.erase();
    }
    const string & get_level_msg() const { return msg_; }

private:
    // where the assertion occured
    string file_;
    int line_;

    // expression and values
    string expr_;
    vals_array vals_;

    // level and message
    int level_;
    string msg_;
};


namespace smart_assert {

    typedef void (*assert_func)( const assert_context & context);

    // helpers
    std::string get_typeof_level( int nLevel);
    void dump_context_summary( const assert_context & context, std::ostream & out);
    void dump_context_detail( const assert_context & context, std::ostream & out);

    // defaults
    void default_warn_handler( const assert_context & context);
    void default_debug_handler( const assert_context & context);
    void default_error_handler( const assert_context & context);
    void default_fatal_handler( const assert_context & context);
    void default_logger( const assert_context & context);

} // namespace smart_assert

namespace Private {
    void init_assert();
    void set_default_log_stream( std::ostream & out);
    void set_default_log_name( const char * str);

    // allows finding if a value is of type 'const char *'
    // and is null; if so, we cannot print it to an ostream
    // directly!!!
    template< class T>
    struct is_null_finder {
        bool is( const T &) const {
            return false;
        }
    };

    template<>
    struct is_null_finder< char*> {
        bool is( char * const & val) {
            return val == 0;
        }
    };

    template<>
    struct is_null_finder< const char*> {
        bool is( const char * const & val) {
            return val == 0;
        }
    };


} // namespace Private


struct Assert {
    typedef smart_assert::assert_func assert_func;

    // helpers, in order to be able to compile the code
    Assert & SMART_ASSERT_A;
    Assert & SMART_ASSERT_B;

    Assert( const char * expr) 
        : SMART_ASSERT_A( *this), 
          SMART_ASSERT_B( *this),
          needs_handling_( true) {
        context_.set_expr( expr);

        if ( ( logger() == 0) || handlers().size() < 4) {
            // used before main!
            Private::init_assert();
        }
    }

    Assert( const Assert & other)
        : SMART_ASSERT_A( *this), 
          SMART_ASSERT_B( *this),
          context_( other.context_),
          needs_handling_( true) {
        other.needs_handling_ = false;
    }

    ~Assert() {
        if ( needs_handling_) 
            handle_assert();
    }

    template< class type>
    Assert & print_current_val( const type & val, const char * msgStr) {
        std::ostringstream out;

        Private::is_null_finder< type> f;
        bool bIsNull = f.is( val);
        if ( !bIsNull)
            out << val;
        else
            // null string
            out << "null";
        context_.add_val( out.str(), msgStr);
        return *this;
    }

    Assert & print_context( const char * file, int line) {
        context_.set_file_line( file, line);
        return *this;
    }

    Assert & msg( const char * strMsg) {
        context_.set_level_msg( strMsg);
        return *this;
    }

    Assert & level( int nLevel, const char * strMsg = 0) {
        context_.set_level( nLevel);
        context_.set_level_msg( strMsg);
        return *this;
    }

    Assert & warn( const char * strMsg = 0) {
        return level( lvl_warn, strMsg);
    }

    Assert & debug( const char * strMsg = 0) {
        return level( lvl_debug, strMsg);
    }

    Assert & error( const char * strMsg = 0) {
        return level( lvl_error, strMsg);
    }

    Assert & fatal( const char * strMsg = 0) {
        return  level( lvl_fatal, strMsg);
    }

    // in this case, we set the default logger, and make it
    // write everything to this file
    static void set_log( const char * strFileName) {
        Private::set_default_log_name( strFileName);
        logger() = &smart_assert::default_logger;
    }

    // in this case, we set the default logger, and make it
    // write everything to this log
    static void set_log( std::ostream & out) {
        Private::set_default_log_stream( out);
        logger() = &smart_assert::default_logger;
    }

    static void set_log( assert_func log) {
        logger() = log;
    }

    static void set_handler( int nLevel, assert_func handler) {
        handlers()[ nLevel] = handler;
    }

private:
    // handles the current assertion.
    void handle_assert() {
        logger()( context_);
        get_handler( context_.get_level() )( context_);
    }

    /*
        IMPORTANT NOTE:
        The only reason logger & handlers are functions, are
        because you might use SMART_ASSERT before main().

        In this case, since they're statics, they might not
        be initialized. However, making them functions 
        will make it work.
    */

    // the log
    static assert_func & logger() {
        static assert_func inst;
        return inst;
    }

    // the handler
    typedef std::map< int, assert_func> handlers_collection; 
    static handlers_collection & handlers() {
        static handlers_collection inst;
        return inst;
    }

    static assert_func get_handler( int nLevel) {
        handlers_collection::const_iterator found = handlers().find( nLevel);
        if ( found != handlers().end() )
            return (*found).second;
        else
            // we always assume the debug handler has been set
            return (*handlers().find( lvl_debug)).second;
    }

private:
    assert_context context_;
    mutable bool needs_handling_;

};


namespace smart_assert {
    inline Assert make_assert( const char * expr) {
        return Assert( expr);
    }
} // namespace smart_assert



////////////////////////////////////////////////////////
// macro trickery

// note: NEVER define SMART_ASSERT_DEBUG directly
// (it will be overridden);
//
// #define SMART_ASSERT_DEBUG_MODE instead

#ifdef SMART_ASSERT_DEBUG_MODE 
    #if SMART_ASSERT_DEBUG_MODE == 1
    #define SMART_ASSERT_DEBUG 
    #else
    #undef SMART_ASSERT_DEBUG
    #endif

#else

// defaults
    #ifndef NDEBUG
    #define SMART_ASSERT_DEBUG 
    #else
    #undef SMART_ASSERT_DEBUG
    #endif
#endif


#ifdef SMART_ASSERT_DEBUG
// "debug" mode
#define SMART_ASSERT( expr) \
    if ( (expr) ) ; \
    else ::smart_assert::make_assert( #expr).print_context( __FILE__, __LINE__).SMART_ASSERT_A \
    /**/

#else
// "release" mode
#define SMART_ASSERT( expr) \
    if ( true ) ; \
    else ::smart_assert::make_assert( "").SMART_ASSERT_A \
    /**/

#endif // ifdef SMART_ASSERT_DEBUG


#define SMART_VERIFY( expr) \
    if ( (expr) ) ; \
    else ::smart_assert::make_assert( #expr).error().print_context( __FILE__, __LINE__).SMART_ASSERT_A \
    /**/


#define SMART_ASSERT_A(x) SMART_ASSERT_OP(x, B)
#define SMART_ASSERT_B(x) SMART_ASSERT_OP(x, A)

#define SMART_ASSERT_OP(x, next) \
    SMART_ASSERT_A.print_current_val((x), #x).SMART_ASSERT_ ## next \
    /**/


#if _MSC_VER > 1000
#pragma warning ( pop )
#endif


#endif 
