
/*
  Copyright 1995 The University of Rhode Island and The Massachusetts
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

// implementation for Grid.
//
// jhrg 9/15/94

// $Log: Grid.cc,v $
// Revision 1.13  1995/08/23 00:11:12  jimg
// Changed old, deprecated member functions to new ones.
// Switched from String representation of type to enum.
//
// Revision 1.12  1995/07/09  21:28:59  jimg
// Added copyright notice.
//
// Revision 1.11  1995/05/10  15:34:00  jimg
// Failed to change `config.h' to `config_dap.h' in these files.
//
// Revision 1.10  1995/05/10  13:45:18  jimg
// Changed the name of the configuration header file from `config.h' to
// `config_dap.h' so that other libraries could have header files which were
// installed in the DODS include directory without overwriting this one. Each
// config header should follow the convention config_<name>.h.
//
// Revision 1.9  1995/03/16  17:29:10  jimg
// Added include config_dap.h to top of include list.
// Added TRACE_NEW switched dbnew includes.
// Fixed bug in read_val() where **val was passed incorrectly to
// subordinate read_val() calls.
//
// Revision 1.8  1995/03/04  14:34:45  jimg
// Major modifications to the transmission and representation of values:
// 	Added card() virtual function which is true for classes that
// 	contain cardinal types (byte, int float, string).
// 	Changed the representation of Str from the C rep to a C++
// 	class represenation.
// 	Chnaged read_val and store_val so that they take and return
// 	types that are stored by the object (e.g., inthe case of Str
// 	an URL, read_val returns a C++ String object).
// 	Modified Array representations so that arrays of card()
// 	objects are just that - no more storing strings, ... as
// 	C would store them.
// 	Arrays of non cardinal types are arrays of the DODS objects (e.g.,
// 	an array of a structure is represented as an array of Structure
// 	objects).
//
// Revision 1.7  1995/02/10  02:23:07  jimg
// Added DBMALLOC includes and switch to code which uses malloc/free.
// Private and protected symbols now start with `_'.
// Added new accessors for name and type fields of BaseType; the old ones
// will be removed in a future release.
// Added the store_val() mfunc. It stores the given value in the object's
// internal buffer.
// Made both List and Str handle their values via pointers to memory.
// Fixed read_val().
// Made serialize/deserialize handle all malloc/free calls (even in those
// cases where xdr initiates the allocation).
// Fixed print_val().
//
// Revision 1.6  1995/01/19  20:05:27  jimg
// ptr_duplicate() mfunc is now abstract virtual.
// Array, ... Grid duplicate mfuncs were modified to take pointers, not
// referenves.
//
// Revision 1.5  1995/01/11  15:54:46  jimg
// Added modifications necessary for BaseType's static XDR pointers. This
// was mostly a name change from xdrin/out to _xdrin/out.
// Removed the two FILE pointers from ctors, since those are now set with
// functions which are friends of BaseType.
//
// Revision 1.4  1994/12/14  20:56:57  dan
// Fixed deserialize() to return correct size count.
// Fixed check_semantics() to use new Array dimension member functions.
//
// Revision 1.3  1994/10/17  23:34:53  jimg
// Added code to print_decl so that variable declarations are pretty
// printed.
// Added private mfunc duplicate().
// Added ptr_duplicate().
// Added Copy ctor, dtor and operator=.
//
// Revision 1.2  1994/09/23  14:45:28  jimg
// Added mfunc check_semantics().
// Added sanity checking on the variable list (is it empty?).
//

#include "config_dap.h"

#include <assert.h>

#include "Grid.h"
#include "Array.h"		// for downcasts
#include "util.h"
#include "errmsg.h"

#ifdef TRACE_NEW
#include "trace_new.h"
#endif

void
Grid::_duplicate(const Grid &s)
{
    set_name(s.name());
    set_type(s.type());
    
    _array_var = s._array_var->ptr_duplicate();

    Grid &cs = (Grid)s;		// cast away const;

    for (Pix p = cs._map_vars.first(); p; cs._map_vars.next(p))
	_map_vars.append(cs._map_vars(p)->ptr_duplicate());
}

Grid::Grid(const String &n) : BaseType(n, grid_t)
{
}

Grid::Grid(const Grid &rhs)
{
    _duplicate(rhs);
}

Grid::~Grid()
{
    delete _array_var;

    for (Pix p = _map_vars.first(); p; _map_vars.next(p))
	delete _map_vars(p);
}

const Grid &
Grid::operator=(const Grid &rhs)
{
    if (this == &rhs)
	return *this;

    _duplicate(rhs);

    return *this;
}

#ifdef NEVER
bool
Grid::card()
{
    return false;
}
#endif

#ifdef NEVER
unsigned int
Grid::size()
{
    return width();
}
#endif

unsigned int
Grid::width()
{
    unsigned int sz = _array_var->width();
  
    for (Pix p = _map_vars.first(); p; _map_vars.next(p)) 
	sz += _map_vars(p)->width();
  
    return sz;
}

bool
Grid::serialize(bool flush)
{
    bool status;

    if (!(status = _array_var->serialize(false))) 
	return false;

    for (Pix p = _map_vars.first(); p; _map_vars.next(p))
	if  (!(status = _map_vars(p)->serialize(false)) ) 
	    break;
	
    if (status && flush)
	status = expunge();

    return status;
}

bool
Grid::deserialize(bool reuse)
{
    unsigned int num, sz = 0;
    
    sz += num = _array_var->deserialize(reuse);
    if (num == 0) 
	return (unsigned int)false;

    for(Pix p = _map_vars.first(); p; _map_vars.next(p)) {
	sz += num = _map_vars(p)->deserialize(reuse);
	if (num == 0) 
	    return (unsigned int)false;
    }

    return sz;
}

unsigned int
Grid::store_val(void *val, bool reuse)
{
    return val2buf(val, reuse);
}

unsigned int
Grid::val2buf(void *val, bool reuse)
{
    assert(val);

    unsigned int pos = 0;
    pos += _array_var->val2buf(val, reuse);

    for(Pix p = _map_vars.first(); p; _map_vars.next(p))
	pos += _map_vars(p)->val2buf(val + pos, reuse);

    return pos;
}

unsigned int
Grid::read_val(void **val)
{
    return buf2val(val);
}

unsigned int
Grid::buf2val(void **val)
{
    assert(val);

    if (!*val)
	*val = new char[width()];

    unsigned int pos = _array_var->buf2val(val);

    for(Pix p = _map_vars.first(); p; _map_vars.next(p)) {
        void *tval = *val + pos;
	pos += _map_vars(p)->buf2val(&tval);
    }

    return pos;
}

BaseType *
Grid::var(const String &name)
{
    if (_array_var->name() == name)
	return _array_var;

    for (Pix p = _map_vars.first(); p; _map_vars.next(p))
	if (_map_vars(p)->name() == name)
	    return _map_vars(p);

    return 0;
}    

void 
Grid::add_var(BaseType *bt, Part part)
{
    switch (part) {
      case array:
	_array_var = bt;
	return;
      case maps:
	_map_vars.append(bt);
	return;
      default:
	err_quit("Grid::add_var:Unknown grid part (must be array or maps)");
	return;
    }
}    

BaseType *
Grid::array_var()
{
    return _array_var;
}

Pix 
Grid::first_map_var()
{
    return _map_vars.first();
}

void 
Grid::next_map_var(Pix &p)
{
    if (!_map_vars.empty() && p)
	_map_vars.next(p);
}

BaseType *
Grid::map_var(Pix p)
{
    if (!_map_vars.empty() && p)
	return _map_vars(p);
}

void 
Grid::print_decl(ostream &os, String space, bool print_semi)
{
    os << space << type_name() << " {" << endl;

    os << space << " ARRAY:" << endl;
    _array_var->print_decl(os, space + "    ");

    os << space << " MAPS:" << endl;
    for (Pix p = _map_vars.first(); p; _map_vars.next(p))
	_map_vars(p)->print_decl(os, space + "    ");

    os << space << "} " << name();
    if (print_semi)
	os << ";" << endl;
}

void 
Grid::print_val(ostream &os, String space, bool print_decl_p)
{
    if (print_decl_p) {
	print_decl(os, space, false);
	os << " = ";
    }

    os << "{ ARRAY: ";
    _array_var->print_val(os, "", false);
    os << " MAPS: ";
    for (Pix p = _map_vars.first(); p; _map_vars.next(p), p && os << ", ")
	_map_vars(p)->print_val(os, "", false);
    os << " }";

    if (print_decl_p)
	os << ";";
}

// Grids have ugly semantics.

bool
Grid::check_semantics(bool all)
{
    if (!BaseType::check_semantics())
	return false;

    if (!unique(_map_vars, (const char *)name(), (const char *)type_name()))
	return false;

    if (!_array_var) {
	cerr << "Null grid base array in `" << name() << "'" << endl;
	return false;
    }
	
    // Is it an array?
    if (_array_var->type() != array_t) {
	cerr << "Grid `" << name() << "'s' member `"
	    << _array_var->name() << "' must be an array" << endl;
	return false;
    }
	    
    Array *av = (Array *)_array_var; // past test above, must be an array

    // enough maps?
    if (_map_vars.length() != av->dimensions()) {
	cerr << "The number of map variables for grid `"
	     << this->name() 
	     << "' does not match the number of dimensions of `"
	    << av->name() << "'" << endl;
	return false;
    }

    const String &array_var_name = av->name();
    Pix p, ap;
    for (p = _map_vars.first(), ap = av->first_dim();
	 p; _map_vars.next(p), av->next_dim(ap)) {

	BaseType *mv = _map_vars(p);

	// check names
	if (array_var_name == mv->name()) {
	    cerr << "Grid map variable `" << mv->name()
		<< "' conflicts with the grid array name in grid `"
		<< name() << "'" << endl;
	    return false;
	}
	// check types
	if (mv->type() != array_t) {
	    cerr << "Grid map variable  `" << mv->name()
		<< "' is not an array" << endl;
	    return false;
	}

	Array *mv_a = (Array *)mv; // downcast to (Array *)

	// check shape
	if (mv_a->dimensions() != 1) {// maps must have one dimension
	    cerr << "Grid map variable  `" << mv_a->name()
		<< "' must be only one dimension" << endl;
	    return false;
	}
	// size of map must match corresponding array dimension
	if (mv_a->dimension_size(mv_a->first_dim()) != av->dimension_size(ap)) {
	    cerr << "Grid map variable  `" << mv_a->name()
		<< "'s' size does not match the size of array variable '"
		<< _array_var->name() << "'s' cooresponding dimension"
		<< endl;
	    return false;
	}
    }

    if (all) {
	if (!_array_var->check_semantics(true))
	    return false;
	for (Pix p = _map_vars.first(); p; _map_vars.next(p))
	    if (!_map_vars(p)->check_semantics(true))
		return false;
    }

    return true;
}

