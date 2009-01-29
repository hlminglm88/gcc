// -*- C++ -*-
// typelist for the C++ library testsuite. 
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.
//
// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#ifndef _TESTSUITE_COMMON_TYPES_H
#define _TESTSUITE_COMMON_TYPES_H 1

#include <ext/typelist.h>

#include <ext/new_allocator.h>
#include <ext/malloc_allocator.h>
#include <ext/mt_allocator.h>
#include <ext/bitmap_allocator.h>
#include <ext/pool_allocator.h>

#include <algorithm>

#include <vector>
#include <list>
#include <deque>
#include <string>

#include <map>
#include <set>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <cstdatomic>
#include <type_traits>
#endif

namespace __gnu_test
{
  using __gnu_cxx::typelist::node;
  using __gnu_cxx::typelist::transform;
  using __gnu_cxx::typelist::append;

  // All the allocators to test.
  template<typename Tp, bool Thread>
    struct allocator_policies
    {
      typedef Tp			    	value_type;
      typedef __gnu_cxx::new_allocator<Tp> 		a1;
      typedef __gnu_cxx::malloc_allocator<Tp> 		a2;
      typedef __gnu_cxx::__common_pool_policy<__gnu_cxx::__pool, Thread> pool_policy;
      typedef __gnu_cxx::__mt_alloc<Tp, pool_policy>	a3;
      typedef __gnu_cxx::bitmap_allocator<Tp> 		a4;
      typedef __gnu_cxx::__pool_alloc<Tp> 		a5;
      typedef node<_GLIBCXX_TYPELIST_CHAIN5(a1, a2, a3, a4, a5)> type;
    };

  // Typelists for vector, string, list, deque.
  // XXX should just use template templates
  template<typename Tp, bool Thread>
    struct vectors
    {
      typedef Tp			    		value_type;

      template<typename Tl>
        struct vector_shell
	{
	  typedef Tl 					allocator_type;
	  typedef std::vector<value_type, allocator_type>	type;
	};

      typedef allocator_policies<value_type, Thread>	allocator_types;
      typedef typename allocator_types::type 		allocator_typelist;
      typedef typename transform<allocator_typelist, vector_shell>::type type;
    };

  template<typename Tp, bool Thread>
    struct lists
    {
      typedef Tp			    		value_type;

      template<typename Tl>
        struct list_shell
	{
	  typedef Tl 					allocator_type;
	  typedef std::list<value_type, allocator_type>	type;
	};

      typedef allocator_policies<value_type, Thread>	allocator_types;
      typedef typename allocator_types::type 		allocator_typelist;
      typedef typename transform<allocator_typelist, list_shell>::type type;
    };

  template<typename Tp, bool Thread>
    struct deques
    {
      typedef Tp			    		value_type;

      template<typename Tl>
        struct deque_shell
	{
	  typedef Tl 					allocator_type;
	  typedef std::deque<value_type, allocator_type>	type;
	};

      typedef allocator_policies<value_type, Thread>	allocator_types;
      typedef typename allocator_types::type 		allocator_typelist;
      typedef typename transform<allocator_typelist, deque_shell>::type type;
    };

  template<typename Tp, bool Thread>
    struct strings
    {
      typedef Tp			    		value_type;

      template<typename Tl>
        struct string_shell
	{
	  typedef Tl 					allocator_type;
	  typedef std::char_traits<value_type> 		traits_type;
	  typedef std::basic_string<value_type, traits_type, allocator_type>	type;
	};

      typedef allocator_policies<value_type, Thread>	allocator_types;
      typedef typename allocator_types::type 		allocator_typelist;
      typedef typename transform<allocator_typelist, string_shell>::type type;
    };

  // A typelist of vector, list, deque, and string all instantiated
  // with each of the allocator policies.
  template<typename Tp, bool Thread>
    struct sequence_containers
    {
      typedef Tp			    		value_type;

      typedef typename vectors<value_type, Thread>::type vector_typelist;
      typedef typename lists<value_type, Thread>::type   list_typelist;
      typedef typename deques<value_type, Thread>::type  deque_typelist;
      typedef typename strings<value_type, Thread>::type string_typelist;

      typedef typename append<vector_typelist, list_typelist>::type a1;
      typedef typename append<deque_typelist, string_typelist>::type a2;
      typedef typename append<a1, a2>::type type;
    };

  // Typelists for map, set, unordered_set, unordered_map.
  template<typename Tp, bool Thread>
    struct maps
    {
      typedef Tp			    		value_type;
      typedef Tp 					key_type;
      typedef std::pair<const key_type, value_type> 	pair_type;
      typedef std::less<key_type>      			compare_function;

      template<typename Tl>
        struct container
	{
	  typedef Tl 					allocator_type;
	  typedef std::map<key_type, value_type, compare_function, allocator_type>	type;
	};

      typedef allocator_policies<pair_type, Thread>	allocator_types;
      typedef typename allocator_types::type 		allocator_typelist;
      typedef typename transform<allocator_typelist, container>::type type;
    };

  template<typename Tp, bool Thread>
    struct unordered_maps
    {
      typedef Tp			    		value_type;
      typedef Tp 					key_type;
      typedef std::pair<const key_type, value_type> 	pair_type;
      typedef std::tr1::hash<key_type>      		hash_function;
      typedef std::equal_to<key_type>      		equality_function;

      template<typename Tl>
        struct container
	{
	  typedef Tl 					allocator_type;
	  typedef std::tr1::unordered_map<key_type, value_type, hash_function, equality_function, allocator_type>	type;
	};

      typedef allocator_policies<pair_type, Thread>	allocator_types;
      typedef typename allocator_types::type 		allocator_typelist;
      typedef typename transform<allocator_typelist, container>::type type;
    };

  template<typename Tp, bool Thread>
    struct sets
    {
      typedef Tp			    		value_type;
      typedef Tp 					key_type;
      typedef std::less<key_type>      			compare_function;

      template<typename Tl>
        struct container
	{
	  typedef Tl 					allocator_type;
	  typedef std::set<key_type, compare_function, allocator_type>	type;
	};

      typedef allocator_policies<key_type, Thread>	allocator_types;
      typedef typename allocator_types::type 		allocator_typelist;
      typedef typename transform<allocator_typelist, container>::type type;
    };

  template<typename Tp, bool Thread>
    struct unordered_sets
    {
      typedef Tp			    		value_type;
      typedef Tp 					key_type;
      typedef std::tr1::hash<key_type>      		hash_function;
      typedef std::equal_to<key_type>      		equality_function;

      template<typename Tl>
        struct container
	{
	  typedef Tl 					allocator_type;
	  typedef std::tr1::unordered_set<key_type, hash_function, equality_function, allocator_type>	type;
	};

      typedef allocator_policies<key_type, Thread>	allocator_types;
      typedef typename allocator_types::type 		allocator_typelist;
      typedef typename transform<allocator_typelist, container>::type type;
    };


  // A typelist  of all associated  container types, with each  of the
  // allocator policies.
  template<typename Tp, bool Thread>
    struct associative_containers
    {
      typedef Tp			    		value_type;

      typedef typename maps<value_type, Thread>::type map_typelist;
      typedef typename sets<value_type, Thread>::type set_typelist;
      typedef typename unordered_maps<value_type, Thread>::type unordered_map_typelist;
      typedef typename unordered_sets<value_type, Thread>::type unordered_set_typelist;

      typedef typename append<map_typelist, unordered_map_typelist>::type a1;
      typedef typename append<set_typelist, unordered_set_typelist>::type a2;
      typedef typename append<a1, a2>::type type;
    };

  // A typelist of all integral types.
  struct integral_types
  {
    typedef bool 		a1;
    typedef char 		a2;
    typedef signed char 	a3;
    typedef unsigned char 	a4;
    typedef short 		a5;
    typedef unsigned short 	a6;
    typedef int 		a7;
    typedef unsigned int 	a8;
    typedef long 		a9;
    typedef unsigned long 	a10;
    typedef long long 		a11;
    typedef unsigned long long 	a12;
    typedef wchar_t 		a13;
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    typedef char16_t 		a14;
    typedef char32_t 		a15;

    typedef node<_GLIBCXX_TYPELIST_CHAIN15(a1, a2, a3, a4, a5, a6, a7, a8, a9, 
					   a10, a11, a12, a13, a14, a15)> type;
#else
    typedef node<_GLIBCXX_TYPELIST_CHAIN13(a1, a2, a3, a4, a5, a6, a7, a8, a9, 
					   a10, a11, a12, a13)> type;
#endif
  };

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  struct atomic_integrals_no_bool
  {
    typedef std::atomic_char        	a2;
    typedef std::atomic_schar 		a3;
    typedef std::atomic_uchar 		a4;
    typedef std::atomic_short       	a5;
    typedef std::atomic_ushort 		a6;
    typedef std::atomic_int 		a7;
    typedef std::atomic_uint 		a8;
    typedef std::atomic_long        	a9;
    typedef std::atomic_ulong 		a10;
    typedef std::atomic_llong       	a11;
    typedef std::atomic_ullong 		a12;
    typedef std::atomic_wchar_t     	a13;
    typedef std::atomic_char16_t    	a14;
    typedef std::atomic_char32_t    	a15;
    
    typedef node<_GLIBCXX_TYPELIST_CHAIN14(a2, a3, a4, a5, a6, a7, a8, a9, 
					   a10, a11, a12, a13, a14, a15)> type;
  };

  struct atomic_integrals
  {
    typedef std::atomic_bool        	a1;
    typedef std::atomic_char        	a2;
    typedef std::atomic_schar 		a3;
    typedef std::atomic_uchar 		a4;
    typedef std::atomic_short       	a5;
    typedef std::atomic_ushort 		a6;
    typedef std::atomic_int 		a7;
    typedef std::atomic_uint 		a8;
    typedef std::atomic_long        	a9;
    typedef std::atomic_ulong 		a10;
    typedef std::atomic_llong       	a11;
    typedef std::atomic_ullong 		a12;
    typedef std::atomic_wchar_t     	a13;
    typedef std::atomic_char16_t    	a14;
    typedef std::atomic_char32_t    	a15;
    
    typedef node<_GLIBCXX_TYPELIST_CHAIN15(a1, a2, a3, a4, a5, a6, a7, a8, a9, 
					   a10, a11, a12, a13, a14, a15)> type;
  };


  template<typename Tp>
    struct atomics
    {
      typedef Tp			value_type;
      typedef std::atomic<value_type>	type;
    };

  typedef transform<integral_types::type, atomics>::type atomics_tl;
#endif


  struct has_increment_operators
  {
    template<typename _Tp>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  {
	    _Tp a; 
	    ++a; // prefix
	    a++; // postfix
	    a += a;
	  }
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  struct has_decrement_operators
  {
    template<typename _Tp>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  {
	    _Tp a; 
	    --a; // prefix
	    a--; // postfix
	    a -= a;
	  }
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  template<typename _Tp>
    void
    bitwise_operators()
    {
      _Tp a; 
      _Tp b;
      a | b;
      a & b;
      a ^ b;
      ~b;
    }

  template<typename _Tp>
    void
    bitwise_assignment_operators()
    {
      _Tp a; 
      _Tp b;
      a |= b; // set
      a &= ~b; // clear
      a ^= b;
    }

  // 17.3.2.1.2 - Bitmask types [lib.bitmask.types]
  // bitmask_operators
  template<typename _BitmTp>
    void
    bitmask_operators()
    {
      bitwise_operators<_BitmTp>();
      bitwise_assignment_operators<_BitmTp>();
    }

  struct has_bitwise_operators
  {
    template<typename _Tp>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  {
	    bitwise_assignment_operators<_Tp>();
	  }
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  // Generator to test standard layout
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  struct has_trivial_cons_dtor
  {
    template<typename _Tp>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  {
	    typedef std::has_trivial_default_constructor<_Tp> ctor_p;
	    static_assert(ctor_p::value, "default constructor not trivial");

	    typedef std::has_trivial_destructor<_Tp> dtor_p;
	    static_assert(dtor_p::value, "destructor not trivial");
	  }
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  struct standard_layout
  {
    template<typename _Tp>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  {
	    // libstdc++/37907
	    // typedef std::is_standard_layout<_Tp> standard_layout_p;
	    // static_assert(standard_layout_p::value, "not standard_layout");

	    typedef std::has_virtual_destructor<_Tp> ctor_p;
	    static_assert(!ctor_p::value, "has virtual destructor");
	  }
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };
#endif

  // Generator to test base class
  struct has_required_base_class
  {
    template<typename _TBase, typename _TDerived>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  {
	    const _TDerived& obj = __a;
	    const _TBase* base __attribute__((unused)) = &obj;
	  }
	  
	  _TDerived __a;
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  // Generator to test assignment operator.
  struct assignable
  {
    template<typename _Tp>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  { __v1 = __v2; }

	  _Tp __v1;
	  _Tp __v2;
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  // Generator to test default constructor.
  struct default_constructible
  {
    template<typename _Tp>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  { _Tp __v; }
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  // Generator to test copy constructor.
  struct copy_constructible
  {
    template<typename _Tp>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  { _Tp __v2(__v1); }

	  _Tp __v1;
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  // Generator to test direct initialization, single value constructor.
  struct single_value_constructible
  {
    template<typename _Ttype, typename _Tvalue>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  { _Ttype __v(__a); }
	  
	  _Tvalue __a;
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  // Generator to test direct list initialization
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  struct direct_list_initializable
  {
    template<typename _Ttype, typename _Tvalue>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  { 
	    _Ttype __v1 { }; // default ctor
	    _Ttype __v2 { __a };  // single-argument ctor
	  }
	  
	  _Tvalue __a;
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };
#endif

  // Generator to test copy list initialization, aggregate initialization
  struct copy_list_initializable
  {
    template<typename _Ttype, typename _Tvalue>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  { _Ttype __v = {__a}; }
	  
	  _Tvalue __a;
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  // Generator to test integral conversion operator
  struct integral_convertable
  {
    template<typename _Ttype, typename _Tvalue>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  {
	    _Tvalue __v0(0);
	    _Tvalue __v1(1);
	    _Ttype __a(__v1);
	    __v0 = __a;

	    bool test __attribute__((unused)) = true;
	    VERIFY( __v1 == __v0 );
	  }
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };

  // Generator to test integral assignment operator 
  struct integral_assignable
  {
    template<typename _Ttype, typename _Tvalue>
      void 
      operator()()
      {
	struct _Concept
	{
	  void __constraint()
	  {
	    _Tvalue __v0(0);
	    _Tvalue __v1(1);
	    _Ttype __a(__v0);
	    __a = __v1;
	    _Tvalue __vr = __a;

	    bool test __attribute__((unused)) = true;
	    VERIFY( __v1 == __vr );
	  }
	};

	void (_Concept::*__x)() __attribute__((unused))
	  = &_Concept::__constraint;
      }
  };
} // namespace __gnu_test
#endif
