
// -*- C++ -*-

// Class for array variables. The dimensions of the array are stored in the
// list SHAPE. 
//
// jhrg 9/6/94

/* $Log: Array.h,v $
/* Revision 1.11  1995/01/18 18:37:54  dan
/* Declared member function 'readVal', defined in dummy_read.cc
/*
 * Revision 1.10  1995/01/11  15:54:40  jimg
 * Added modifications necessary for BaseType's static XDR pointers. This
 * was mostly a name change from xdrin/out to _xdrin/out.
 * Removed the two FILE pointers from ctors, since those are now set with
 * functions which are friends of BaseType.
 *
 * Revision 1.9  1994/12/14  20:36:58  dan
 * Added dimensions() member function definition.
 * Removed alloc_buf() and free_buf() definitions.
 *
 * Revision 1.8  1994/12/14  17:55:23  dan
 * Removed dim() member function.
 *
 * Revision 1.7  1994/12/12  20:33:34  jimg
 * Fixed struct dimensions.
 *
 * Revision 1.6  1994/12/12  19:40:30  dan
 * Modified Array class definition removing inheritance from class CtorType
 * and to inherit directly from class BaseType.  Removed member function
 * dimensions().
 *
 * Revision 1.5  1994/12/09  21:36:34  jimg
 * Added support for named array dimensions.
 *
 * Revision 1.4  1994/11/22  14:05:22  jimg
 * Added code for data transmission to parts of the type hierarchy. Not
 * complete yet.
 * Fixed erros in type hierarchy headers (typos, incorrect comments, ...).
 *
 * Revision 1.3  1994/10/17  23:34:43  jimg
 * Added code to print_decl so that variable declarations are pretty
 * printed.
 * Added private mfunc duplicate().
 * Added ptr_duplicate().
 * Added Copy ctor, dtor and operator=.
 *
 * Revision 1.2  1994/09/23  14:31:37  jimg
 * Added check_semantics mfunc.
 * Added sanity checking for access to shape list (is it empty?).
 * Added cvs log listing to Array.cc.
 */

#ifndef _Array_h
#define _Array_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include <limits.h>
#include <SLList.h>
#include "BaseType.h"

#include "config.h"
#ifdef TRACE_NEW
#include "trace_new.h"
#endif

const int DODS_MAX_ARRAY = UINT_MAX;

class Array: public BaseType {
private:
    struct dimension {			// each dimension has a size and a name
	int size;
	String name;
    };

    BaseType *var_ptr;		// var that is an array
    SLList<dimension> shape;	// list of dimensions (i.e., the shape)

    void duplicate(const Array &a);
    void *buf;

public:
    Array(const String &n = (char *)0, BaseType *v = 0);
    Array(const Array &rhs);
    virtual ~Array();

    const Array &operator=(const Array &rhs);
    virtual BaseType *ptr_duplicate(); // alloc new Array and dup THIS to it

    virtual unsigned int size();

    virtual bool read(String dataset, String var_name, String constraint);
    virtual bool readVal(void *stuff);

    virtual bool serialize(bool flush, unsigned int num = 0);
    virtual unsigned int deserialize();

    virtual BaseType *var(const String &name = (char *)0);
    virtual void add_var(BaseType *v, Part p = nil);

    void append_dim(int size, String name = "");

    Pix first_dim();
    void next_dim(Pix &p);
    int dimension_size(Pix p);
    String dimension_name(Pix p);
    unsigned int dimensions();

    virtual void print_decl(ostream &os, String space = "    ",
			    bool print_semi = true);
    virtual void print_val(ostream &os, String space = "");

    virtual bool check_semantics(bool all = false);
};

#endif


