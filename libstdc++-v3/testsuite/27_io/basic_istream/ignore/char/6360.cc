// 1999-08-11 bkoz

// Copyright (C) 1999-2015 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// 27.6.1.3 unformatted input functions

#include <istream>
#include <sstream>
#include <testsuite_hooks.h>

// 2002-04-19 PR libstdc++ 6360
void
test08()
{
  using namespace std;
  bool test __attribute__((unused)) = true;

  stringstream ss("abcd" "\xFF" "1234ina donna coolbrith");  
  char c;
  ss >> c;
  VERIFY( c == 'a' );
  ss.ignore(8);
  ss >> c;
  VERIFY( c == 'i' );
}

int 
main()
{
  test08();
  return 0;
}
