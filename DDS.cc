
/*
  Copyright 1994, 1995 The University of Rhode Island and The Massachusetts
  Institute of Technology

  Portions of this software were developed by the Graduate School of
  Oceanography (GSO) at the University of Rhode Island (URI) in collaboration
  with The Massachusetts Institute of Technology (MIT).

  Access and use of this software shall impose the following obligations and
  understandings on the user. The user is granted the right, without any fee
  or cost, to use, copy, modify, alter, enhance and distribute this software,
  and any derivative works thereof, and its supporting documentation for any
  purpose whatsoever, provided that this entire notice appears in all copies
  of the software, derivative works and supporting documentation.  Further,
  the user agrees to credit URI/MIT in any publications that result from the
  use of this software or in any product that includes this software. The
  names URI, MIT and/or GSO, however, may not be used in any advertising or
  publicity to endorse or promote any products or commercial entity unless
  specific written permission is obtained from URI/MIT. The user also
  understands that URI/MIT is not obligated to provide the user with any
  support, consulting, training or assistance of any kind with regard to the
  use, operation and performance of this software nor to provide the user
  with any updates, revisions, new versions or "bug fixes".

  THIS SOFTWARE IS PROVIDED BY URI/MIT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL URI/MIT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
  DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
  PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS
  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE ACCESS, USE OR PERFORMANCE
  OF THIS SOFTWARE.
*/

// Methods for class DDS
//
// jhrg 9/7/94

// $Log: DDS.cc,v $
// Revision 1.14  1995/12/06 21:11:24  jimg
// Added print_constrained(): Prints a constrained DDS.
// Added eval_constraint(): Evaluates a constraint expression in the environment
// of the current DDS.
// Added send(): combines reading, serailizing and constraint evaluation.
// Added mark*(): used to mark variables as part of the current projection.
// Fixed some of the parse() and print() mfuncs to take uniform parameter types
// (ostream and FILE *).
// Fixed the constructors to work with const objects.
//
// Revision 1.13  1995/10/23  23:20:50  jimg
// Added _send_p and _read_p fields (and their accessors) along with the
// virtual mfuncs set_send_p() and set_read_p().
//
// Revision 1.12  1995/08/23  00:06:30  jimg
// Changed from old mfuncs to new(er) ones.
//
// Revision 1.11  1995/07/09  21:28:55  jimg
// Added copyright notice.
//
// Revision 1.10  1995/05/10  13:45:13  jimg
// Changed the name of the configuration header file from `config.h' to
// `config_dap.h' so that other libraries could have header files which were
// installed in the DODS include directory without overwriting this one. Each
// config header should follow the convention config_<name>.h.
//
// Revision 1.9  1994/12/09  21:37:24  jimg
// Added <unistd.h> to the include files.
//
// Revision 1.8  1994/12/07  21:23:16  jimg
// Removed config
//
// Revision 1.7  1994/11/22  14:05:40  jimg
// Added code for data transmission to parts of the type hierarchy. Not
// complete yet.
// Fixed erros in type hierarchy headers (typos, incorrect comments, ...).
//
// Revision 1.6  1994/11/03  04:58:02  reza
// Added two overloading for function parse to make it consistent with DAS class.
//
// Revision 1.5  1994/10/18  00:20:46  jimg
// Added copy ctor, dtor, duplicate, operator=.
// Added var() for const cahr * (to avoid confusion between char * and
// Pix (which is void *)).
// Switched to errmsg library.
// Added formatting to print().
//
// Revision 1.4  1994/10/05  16:34:14  jimg
// Fixed bug in the parse function(s): the bison generated parser returns
// 1 on error, 0 on success, but parse() was not checking for this.
// Instead it returned the value of bison's parser function.
// Changed types of `status' in print and parser functions from int to bool.
//
// Revision 1.3  1994/09/23  14:42:22  jimg
// Added mfunc check_semantics().
// Replaced print mfunc stub with real code.
// Fixed some errors in comments.
//
// Revision 1.2  1994/09/15  21:08:39  jimg
// Added many classes to the BaseType hierarchy - the complete set of types
// described in the DODS API design documet is now represented.
// The parser can parse DDS files.
// Fixed many small problems with BaseType.
// Added CtorType.
//
// Revision 1.1  1994/09/08  21:09:40  jimg
// First version of the Dataset descriptor class.
// 

static char rcsid[]="$Id: DDS.cc,v 1.14 1995/12/06 21:11:24 jimg Exp $";

#ifdef __GNUG__
#pragma implementation
#endif

#include <unistd.h>
#include <stdio.h>

#include <iostream.h>
#include <stdiostream.h>

#include "DDS.h"
#include "errmsg.h"
#include "debug.h"
#include "util.h"

#include "config_dap.h"
#ifdef TRACE_NEW
#include "trace_new.h"
#endif

void ddsrestart(FILE *yyin);
int ddsparse(DDS &table);	// defined in dds.tab.c

void exprrestart(FILE *yyin);
int exprparse(DDS & table);

// Copy the stuff in DDS to THIS. The mfunc returns void because THIS gets
// the `result' of the mfunc.
//
// NB: This can't define the formal param to be const since SLList<>first()
// (which is what DDS::first_var() calls) does not define THIS to be const.

void
DDS::duplicate(const DDS &dds)
{
    name = dds.name;

    DDS &dds_tmp = (DDS &)dds;	// cast away const

    // copy the things pointed to by the list, not just the pointers
    for (Pix src = dds_tmp.first_var(); src; dds_tmp.next_var(src)) {
	BaseType *btp = dds_tmp.var(src)->ptr_duplicate();
	add_var(btp);
    }
}

DDS::DDS(const String &n) : name(n)
{
}

DDS::DDS(const DDS &rhs)
{
    duplicate(rhs);
}

DDS::~DDS()
{
    for (Pix p = first_var(); p; next_var(p))
	delete var(p);
}

DDS &
DDS::operator=(const DDS &rhs)
{
    if (this == &rhs)
	return *this;

    duplicate(rhs);

    return *this;
}

String 
DDS::get_dataset_name()
{ 
    return name; 
}

void
DDS::set_dataset_name(const String &n) 
{ 
    name = n; 
}

void
DDS::add_var(BaseType *bt)
{ 
    vars.append(bt); 
}

void 
DDS::del_var(const String &n)
{ 
    Pix pp = 0;			// previous Pix

    for (Pix p = vars.first(); p; vars.next(p))
	if (vars(p)->name() == n) {
	    vars.del_after(pp);	// pp points to the pos before p
	    return;
	}
	else
	    pp = p;
}

// Return a porinter to the named variable. This mfunc does some unusual
// things: if a name contains one or more `.'s then it assumes that N is the
// name of some aggregate member - it searches for the Structure, ..., Grid
// member named and returns a pointer to that member (regardless of nesting
// level). In addition, if simple name is given, but that name does not
// appear in the list of top-level variables, var() will search aggregates
// for a field by that name and return a pointer to it iff that field name is
// unique.
// 
// Returns: A baseType * to a variable whose name in N. If no such variable
// can be found, returns null.

BaseType *
DDS::var(const String &n)
{
    if (n.contains(".")) {
	String name = (String)n; // cast away const
	String aggregate = name.before(".");
	String field = name.from(".");
	field = field.after(".");

	BaseType *agg_ptr = var(aggregate);
	if (agg_ptr)
	    return agg_ptr->var(field);	// recurse
	else
	    return 0;		// qualified names must be *fully* qualified
    }
    else {
	for (Pix p = vars.first(); p; vars.next(p)) {

	    // Look for the name in the dataset's top-level
	    if (vars(p)->name() == n) {
		DBG(cerr << "Found " << n);
		return vars(p);
	    }

#ifdef NEVER
	    // otherwise, see if it is part of an aggregate
	    switch(vars(p)->type()) {
		BaseType *variable;
	      case array_t:
	      case list_t:
	      case structure_t:
	      case sequence_t:
	      case function_t:
	      case grid_t:

		if ((variable = vars(p)->var(n)))
		    return variable;
		else
		    return 0;	// not found
		break;

	      default:
		return 0;
		break;
	    }
#endif
	}
    }

    return 0;			// It is not here.
}

// This is necessary because (char *) can be cast to Pix (because PIX is
// really (void *)). This must take precedence over the creation of a
// temporary object (the String).

BaseType *
DDS::var(const char *n)
{
    return var((String)n);
}

Pix 
DDS::first_var()
{ 
    return vars.first(); 
}

void 
DDS::next_var(Pix &p)
{ 
    if (!vars.empty())
	vars.next(p); 
}

BaseType *
DDS::var(Pix p)
{ 
    if (!vars.empty() && p)
	return vars(p); 
}

bool
DDS::parse(String fname)
{
    FILE *in = fopen(fname, "r");

    if (!in) {
        cerr << "Could not open: " << fname << endl;
        return false;
    }

    bool status = parse(in);

    fclose(in);

    return status;
}


bool
DDS::parse(int fd)
{
    FILE *in = fdopen(dup(fd), "r");

    if (!in) {
        cerr << "Could not access file" << endl;
        return false;
    }

    bool status = parse(in);

    fclose(in);

    return status;
}

// Read structure from IN (which defaults to stdin). If ddsrestart() fails,
// return false, otherwise return the status of ddsparse().

bool
DDS::parse(FILE *in)
{
    if (!in) {
	err_print("DDS::parse: NULL file pointer");
	return false;
    }

    ddsrestart(in);

    bool status = ddsparse(*this) == 0;

    fclose(in);

    return status;
}

// Write strucutre from tables to OUT (which defaults to stdout). 
//
// Returns true. 

bool
DDS::print(ostream &os)
{
    os << "Dataset {" << endl;

    for (Pix p = vars.first(); p; vars.next(p))
	vars(p)->print_decl(os);

    os << "} " << name << ";" << endl;
					   
    return true;
}

bool 
DDS::print(FILE *out)
{
    ostdiostream os(out);
    return print(os);
}

// Print those parts (variables) of the DDS structure to OS that are marked
// to be sent after evaluating the constraint expression.
//
// NB: this function only works for scalars at the top level.
//
// Returns true.

bool
DDS::print_constrained(ostream &os)
{
    os << "Dataset {" << endl;

    for (Pix p = vars.first(); p; vars.next(p))
	// for each variable, indent with four spaces, print a trailing
	// semi-colon, do not print debugging information, print only
	// variables in the current projection.
	vars(p)->print_decl(os, "    ", true, false, true);

    os << "} " << name << ";" << endl;
					   
    return true;
}

bool
DDS::print_constrained(FILE *out)
{
    ostdiostream os(out);
    return print_constrained(os);
}

// Check the semantics of the DDS describing a complete dataset. If ALL is
// true, check not only the semantics of THIS->TABLE, but also recurrsively
// all ctor types in the THIS->TABLE. By default, ALL is false since parsing
// a DDS input file runns semantic checks on all variables (but not the
// dataset itself.
//
// Returns: true if the conventions for the DDS are not violated, false
// otherwise. 

bool
DDS::check_semantics(bool all)
{
    // The dataset must have a name
    if (name == (char *)0) {
	cerr << "A dataset must have a name" << endl;
	return false;
    }

    if (!unique(vars, (const char *)name, (const char *)"Dataset"))
	return false;

    if (all) 
	for (Pix p = vars.first(); p; vars.next(p))
	    if (!vars(p)->check_semantics(true))
		return false;

    return true;
}

// Evaluate the constraint expression; return the value of the expression. As
// a side effect, mark the DDS so that BaseType's mfuncs can be used to
// correctly read the variable's value and send it to the client.
//
// Returns: true if the constraint expression is true for the current DDS,
// false otherwise.

bool
DDS::eval_constraint(String constraint)
{
    FILE *in = text_to_temp(constraint);

    exprrestart(in);
    
    return exprparse(*this) == 0; // status == 0 indicates success
}

// Send the named variable. This mfunc combines BaseTypes read() and
// serialize() mfuncs. It also ensures that the data (binary) is prefixed
// with a DDS which describes the binary data.
//
// NB: FLUSH defaults to false.
//
// Returns: true if successful, false otherwise.

bool 
DDS::send(String dataset, String var_name, String constraint, bool flush,
	  FILE *out)
{
    bool status = true;

    // Evaluate the constraint expression. This changes the DDS so that
    // BaseType's read() and serialize() mfuncs will do the rgiht thing.

    if ((status = eval_constraint(constraint))) {
	BaseType *variable = var(var_name);

	if (variable && variable->send_p()) { // only send if necessary
	    if (!variable->read_p()) // only read if necessary
		status = variable->read(dataset, var_name);

	    if (status) {
		// set up output stream
		ostdiostream os(out);

		// send constrained DDS for this variable
		print_constrained(os);

		// send `Data:' marker
		os << "Data:" << endl;

		// send binary data
		set_xdrout(out);
		status = variable->serialize(flush);
	    }
	}
    }

    return status;
}

// Mark the named variable by setting its send_p flag to state (true
// indicates that it is to be sent). Names must be fully qualified.
//
// Returns: True if the named variable was found, false otherwise.

bool
DDS::mark(const String &n, bool state)
{
    if (n.contains(".")) {
	String field = (String &)n; // cast away const

	String aggregate = field.before(".");
	BaseType *variable = var(aggregate); // get first variable from DDS
	if (!variable)
	    return false;	// no such variable
	else if (state)
	    variable->BaseType::set_send_p(state); // set iff state == true
	field = field.after(".");

	while (field.contains(".")) {
	    aggregate = field.before(".");
	    variable = variable->var(aggregate); // get child var using parent
	    if (!variable)
		return false;	// no such variable
	    else if (state)
		variable->BaseType::set_send_p(state); // set iff state == true
	    field = field.after(".");
	}

	variable->var(field)->set_send_p(state); // set last child

	return true;		// marked field and its parents
    }
    else {
	BaseType *variable = var(n);
	if (!variable) {
	    DBG(cerr << "Could not find variable " << n << endl);
	    return false;
	}
	variable->set_send_p(state);

	return true;
    }

    return false;		// not found
}

bool
DDS::mark_all(bool state)
{
    for (Pix p = first_var(); p; next_var(p))
	var(p)->set_send_p(state);
}
    
