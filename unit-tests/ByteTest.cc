
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
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

#include "config.h"

#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

// Tests for Byte. Tests features of BaseType, too. 7/19/2001 jhrg

#include <sstream>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "Byte.h"

#include "testFile.cc"

using namespace CppUnit;
using namespace std;
using namespace libdap;

class ByteTest:public TestFixture {
  private:
    Byte * tb1;
    Byte *tb2;
    Byte *tb3;
    Byte *tb4;

  public:
     ByteTest() {
    } ~ByteTest() {
    }

    void setUp() {
        tb1 = new Byte("tb1");
        tb2 = new Byte("tb2 name with spaces");
        tb3 = new Byte("tb3 %");
        tb4 = new Byte("tb4 #");
    }

    void tearDown() {
        delete tb1;
        tb1 = 0;
        delete tb2;
        tb2 = 0;
        delete tb3;
        tb3 = 0;
        delete tb4;
        tb4 = 0;
    }

    CPPUNIT_TEST_SUITE(ByteTest);

    CPPUNIT_TEST(name_mangling_test);
    CPPUNIT_TEST(decl_mangling_test);

    CPPUNIT_TEST_SUITE_END();

    void name_mangling_test() {
        CPPUNIT_ASSERT(tb1->name() == "tb1");
        CPPUNIT_ASSERT(tb2->name() == "tb2 name with spaces");
        CPPUNIT_ASSERT(tb3->name() == "tb3 %");
        CPPUNIT_ASSERT(tb4->name() == "tb4 #");
    }

    void decl_mangling_test() {
        string sof;
        FILE2string(sof, of, tb1->print_decl(of, "", false));
        CPPUNIT_ASSERT(sof.find("Byte tb1") != string::npos);

        FILE2string(sof, of, tb2->print_decl(of, "", false));
        CPPUNIT_ASSERT(sof.find("Byte tb2%20name%20with%20spaces") != string::npos);

        FILE2string(sof, of, tb3->print_decl(of, "", false));
        CPPUNIT_ASSERT(sof.find("Byte tb3%20%") != string::npos);

        FILE2string(sof, of, tb4->print_decl(of, "", false));
        CPPUNIT_ASSERT(sof.find("Byte tb4%20%23") != string::npos);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ByteTest);

int main(int, char **)
{
    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

    bool wasSuccessful = runner.run("", false);

    return wasSuccessful ? 0 : 1;
}
