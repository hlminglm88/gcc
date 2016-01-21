// 2003-09-22  Petur Runolfsson  <peturr02@ru.is>

// Copyright (C) 2003-2016 Free Software Foundation, Inc.
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

// 27.6.2.6 Unformatted output functions
//
// _GLIBCXX_RESOLVE_LIB_DEFECTS
// DR 60. What is a formatted input function?
// basic_ostream::flush() does not behave as an unformatted output function.

#include <ostream>
#include <testsuite_hooks.h>
#include <testsuite_io.h>

void test02()
{
  bool test __attribute__((unused)) = true;

  __gnu_test::sync_streambuf buf;
  std::ostream os(&buf);
  
  __gnu_test::sync_streambuf buf_tie;
  std::ostream os_tie(&buf_tie);

  // No sentry should be constructed so os.tie()->flush() should not be
  // called.
  os.tie(&os_tie);
  
  os.flush();

  VERIFY( os.good() );
  VERIFY( buf.sync_called() );
  VERIFY( !buf_tie.sync_called() );

  // os.rdbuf()->pubsync() should be called even if !os.good().
  os.setstate(std::ios_base::eofbit);

  os.flush();

  VERIFY( os.rdstate() == std::ios_base::eofbit );
  VERIFY( buf.sync_called() );
  VERIFY( !buf_tie.sync_called() );
}

int main()
{
  test02();
  return 0;
}

