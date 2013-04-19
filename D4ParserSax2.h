
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2012 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#ifndef d4_parser_sax2_h
#define d4_parser_sax2_h

#define ATTR 1

#include <string>
#include <iostream>
#include <map>
#include <stack>

#include <libxml/parserInternals.h>

namespace libdap
{

class DMR;
class BaseType;
class D4BaseTypeFactory;

/** Parse the XML text which encodes the network/persistent representation of
    the DMR object. In the current implementation, the DMR is held by an
    instance of the class DDS which in turn holds variables which include
    attributes.

    This parser for the DMR document uses the SAX interface of libxml2.
    Static methods are used as callbacks for the SAX parser. These static
    methods are public because making them private complicates compilation.
    They should not be called by anything other than the intern method.
    They do not throw exceptions because exceptions from within callbacks are
    not reliable or portable. To signal errors, the methods record
    information in the D4ParserSax2 object. Once the error handler is called,
    construction of an DMR object ends even though the SAX parser still
    calls the various callback functions. The parser treats warnings,
    errors and fatal_errors the same way; when any are found parsing
    stops. The intern method throws an Error of InternalErr exception if an
    error was found.

    Note that this class uses the C++-supplied default definitions for the
    default and copy constructors as well as the destructor and assignment
    operator.

    @see DMR */
class D4ParserSax2
{
private:
    /** States used by DDXParserDAP4State. These are the states of the SAX parser
    state-machine. */
    enum ParseState {
        parser_start,

        inside_dataset,

        // inside_group is the state just after parsing the start of a Group
        // element.
        inside_group,

        inside_attribute_container,
        inside_attribute,
        inside_attribute_value,
        inside_other_xml_attribute,

        inside_enum_def,
        inside_enum_const,

        inside_dim_def,

        // This covers Byte, ..., Url, Opaque
        inside_simple_type,

        inside_array,
        inside_dimension,

        inside_structure,

        parser_unknown,
        parser_error,
        parser_fatal_error
    };

    // The results of the parse operation are stored in these fields.
    // This is passed into the parser using the intern() methods.
    DMR *d_dmr;   // dump DMR here
    DMR *dmr() const { return d_dmr; }

    // These stacks hold the state of the parse as it progresses.

    stack<ParseState> s; // Current parse state
    void push_state(D4ParserSax2::ParseState state) { s.push(state); }
    D4ParserSax2::ParseState get_state() const { return s.top(); }
    void pop_state() { s.pop(); }

    stack<BaseType*> btp_stack; // current variable(s)/groups(s)
    void push_basetype(BaseType *btp) { btp_stack.push(btp); }
    BaseType * top_basetype() const { return btp_stack.top(); }
    void pop_basetype() { btp_stack.pop(); }

    stack<D4Attributes*> d_attrs_stack; // DAP4 Attributes
    void push_attributes(D4Attributes *attr) { d_attrs_stack.push(attr); }
    D4Attributes *top_attributes() const { return d_attrs_stack.top(); }
    void pop_attributes() { d_attrs_stack.pop(); }

    D4EnumDef *d_enum_def;
    D4EnumDef *enum_def() {
        if (!d_enum_def) d_enum_def = new D4EnumDef;
        return d_enum_def;
    }
    void clear_enum_def() { d_enum_def = 0; }

    D4Dimension *d_dim_def;
    D4Dimension *dim_def() {
        if (!d_dim_def) d_dim_def = new D4Dimension;
        return d_dim_def;
    }
    void clear_dim_def() { d_dim_def = 0; }

    // Accumulate stuff inside an 'OtherXML' DAP attribute here
    string other_xml;

    // When we're parsing unknown XML, how deeply is it nested? This is used
    // for the OtherXML DAP attributes.
    unsigned int other_xml_depth;
    unsigned int unknown_depth;

    // These are used for processing errors.
    string error_msg;  // Error message(s), if any.
    xmlParserCtxtPtr context; // used for error message line numbers

    // These hold temporary values read during the parse.
    string dods_attr_name; // DAP4 attributes, not XML attributes
    string dods_attr_type; // ... not XML ...
    string char_data;  // char data in value elements; null after use
    string root_ns;     // What is the namespace of the root node (Group)

    bool d_debug;
    bool debug() const { return d_debug; }

    class XMLAttribute {
        public:
        string prefix;
        string nsURI;
        string value;

        void clone(const XMLAttribute &src) {
            prefix = src.prefix;
            nsURI = src.nsURI;
            value = src.value;
        }

        XMLAttribute() : prefix(""), nsURI(""), value("") {}
        XMLAttribute(const string &p, const string &ns, const string &v)
            : prefix(p), nsURI(ns), value(v) {}
        // 'attributes' as passed from libxml2 is a five element array but this
        // ctor gets the back four elements.
        XMLAttribute(const xmlChar **attributes/*[4]*/) {
            prefix = attributes[0] != 0 ? (const char *)attributes[0]: "";
            nsURI = attributes[1] != 0 ? (const char *)attributes[1]: "";
            value = string((const char *)attributes[2], (const char *)attributes[3]);
        }
        XMLAttribute(const XMLAttribute &rhs) {
            clone(rhs);
        }
        XMLAttribute &operator=(const XMLAttribute &rhs) {
            if (this == &rhs)
                return *this;
            clone(rhs);
            return *this;
        }
    };

    typedef map<string, XMLAttribute> XMLAttrMap;
    XMLAttrMap xml_attrs; // dump XML attributes here

    XMLAttrMap::iterator xml_attr_begin() { return xml_attrs.begin(); }

    XMLAttrMap::iterator xml_attr_end() {  return xml_attrs.end(); }

    map<string, string> namespace_table;

    // Common cleanup code for intern()
    void delete_parser_locals();
    void cleanup_parse();

    /** @name Parser Actions

    These methods are the 'actions' carried out by the start_element and
    end_element callbacks. Most of what takes place in those has been
    factored out to this set of functions. */
    //@{
    void transfer_xml_attrs(const xmlChar **attrs, int nb_attributes);
    void transfer_xml_ns(const xmlChar **namespaces, int nb_namespaces);
    bool check_required_attribute(const string &attr);
    bool check_attribute(const string & attr);
    void process_variable_helper(Type t, ParseState s, const xmlChar **attrs, int nb_attributes);

    void process_enum_const_helper(const xmlChar **attrs, int nb_attributes);
    void process_enum_def_helper(const xmlChar **attrs, int nb_attributes);

    bool process_dimension(const char *name, const xmlChar **attrs, int nb_attrs);
    bool process_dimension_def(const char *name, const xmlChar **attrs, int nb_attrs);
    bool process_attribute(const char *name, const xmlChar **attrs, int nb_attributes);
    bool process_variable(const char *name, const xmlChar **attrs, int nb_attributes);
    bool process_group(const char *name, const xmlChar **attrs, int nb_attributes);
    bool process_enum_def(const char *name, const xmlChar **attrs, int nb_attributes);
    bool process_enum_const(const char *name, const xmlChar **attrs, int nb_attributes);

    void finish_variable(const char *tag, Type t, const char *expected);
    //@}

    friend class D4ParserSax2Test;

public:
    D4ParserSax2() :
        d_dmr(0), d_enum_def(0), d_dim_def(0),
        other_xml(""), other_xml_depth(0), unknown_depth(0),
        error_msg(""), context(0),
        dods_attr_name(""), dods_attr_type(""),
        char_data(""), root_ns("")
    {}

    void intern(const string &document, DMR *dest_dmr, bool debug = false);
    void intern(istream &in, DMR *dest_dmr, bool debug = false);

    static void dmr_start_document(void *parser);
    static void dmr_end_document(void *parser);

    static void dmr_start_element(void *parser,
            const xmlChar *localname, const xmlChar *prefix, const xmlChar *URI,
            int nb_namespaces, const xmlChar **namespaces, int nb_attributes,
            int nb_defaulted, const xmlChar **attributes);
    static void dmr_end_element(void *parser, const xmlChar *localname,
            const xmlChar *prefix, const xmlChar *URI);

    static void dmr_get_characters(void *parser, const xmlChar *ch, int len);
    static void dmr_ignoreable_whitespace(void *parser,
            const xmlChar * ch, int len);
    static void dmr_get_cdata(void *parser, const xmlChar *value, int len);

    static xmlEntityPtr dmr_get_entity(void *parser, const xmlChar *name);
    static void dmr_fatal_error(void *parser, const char *msg, ...);
    static void dmr_error(void *parser, const char *msg, ...);
};

} // namespace libdap

#endif // d4_parser_sax2_h
