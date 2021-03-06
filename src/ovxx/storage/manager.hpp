//
// Copyright (c) 2013 Stefan Seefeld
// All rights reserved.
//
// This file is part of OpenVSIP. It is made available under the
// license contained in the accompanying LICENSE.BSD file.

#ifndef ovxx_storage_manager_hpp_
#define ovxx_storage_manager_hpp_

#include <ovxx/storage/storage.hpp>
#include <ovxx/storage/host.hpp>
#include <ovxx/storage/user.hpp>
#if OVXX_HAVE_OPENCL
# include <ovxx/opencl/storage.hpp>
#endif
#include <ovxx/block_traits.hpp>

namespace ovxx
{

// A storage manager handles memory associated with a particular block.
// It knows about the different address spaces managed by
// the memory manager, and replicates data between them accordingly.
template <typename T, storage_format_type F = array>
class storage_manager
{
  enum private_type {};
  typedef typename detail::complex_value_type<T, private_type>::type uT;
  typedef storage_traits<T, F> t;
public:
  typedef typename t::value_type value_type;
  typedef typename t::ptr_type ptr_type;
  typedef typename t::const_ptr_type const_ptr_type;
  typedef typename t::reference_type reference_type;
  typedef typename t::const_reference_type const_reference_type;

  // location identifies an address space.
  // Typically this is a device, but it could also be different
  // memory types from different allocators.
  // The '0' location is the default location, in host memory.
  // It is treated special, as that is the location used by
  // user-storage.
  typedef int location;

  storage_manager(length_type size, bool allocate = true)
    : use_user_storage_(false),
      host_storage_(allocator::get_default(), size, allocate),
#if OVXX_HAVE_OPENCL
      opencl_storage_(size, false),
#endif
      admitted_(true)
  {
  }
  storage_manager(allocator *a, length_type size, bool allocate = true)
    : use_user_storage_(false),
      host_storage_(a, size, allocate),
#if OVXX_HAVE_OPENCL
      opencl_storage_(size, false),
#endif
      admitted_(true)
  {
  }
  // User-storage constructors.
  storage_manager(length_type size, T *ptr)
    : user_storage_(ptr, size),
      use_user_storage_(t::is_compatible(user_storage_.format())),
      host_storage_(allocator::get_default(), size,
		    use_user_storage_ ?
		    user_storage_.template as<F>() :
		    ptr_type()),
#if OVXX_HAVE_OPENCL
      opencl_storage_(size),
#endif
      admitted_(false)
  {
  }
  storage_manager(allocator *a, length_type size, T *ptr)
    : user_storage_(ptr, size),
      use_user_storage_(t::is_compatible(user_storage_.format())),
      host_storage_(a, size,
		    use_user_storage_ ?
		    user_storage_.template as<F>() :
		    ptr_type()),
#if OVXX_HAVE_OPENCL
      opencl_storage_(size),
#endif
      admitted_(false)
  {
  }
  storage_manager(length_type size, uT *ptr)
    : user_storage_(ptr, size),
      use_user_storage_(t::is_compatible(user_storage_.format())),
      host_storage_(allocator::get_default(), size,
		    use_user_storage_ ?
		    user_storage_.template as<F>() :
		    ptr_type()),
#if OVXX_HAVE_OPENCL
      opencl_storage_(size),
#endif
      admitted_(false)
  {
  }
  storage_manager(allocator *a, length_type size, uT *ptr)
    : user_storage_(ptr, size),
      use_user_storage_(t::is_compatible(user_storage_.format())),
      host_storage_(a, size,
		    use_user_storage_ ?
		    user_storage_.template as<F>() :
		    ptr_type()),
#if OVXX_HAVE_OPENCL
      opencl_storage_(size),
#endif
      admitted_(false)
  {
  }
  storage_manager(length_type size, std::pair<uT*,uT*> ptr)
    : user_storage_(ptr, size),
      use_user_storage_(t::is_compatible(user_storage_.format())),
      host_storage_(allocator::get_default(), size,
		    use_user_storage_ ?
		    user_storage_.template as<F>() :
		    ptr_type()),
#if OVXX_HAVE_OPENCL
      opencl_storage_(size),
#endif
      admitted_(false)
  {
  }
  storage_manager(allocator *a, length_type size, std::pair<uT*,uT*> ptr)
    : user_storage_(ptr, size),
      use_user_storage_(t::is_compatible(user_storage_.format())),
      host_storage_(a, size,
		    use_user_storage_ ?
		    user_storage_.template as<F>() :
		    ptr_type()),
#if OVXX_HAVE_OPENCL
      opencl_storage_(size),
#endif
      admitted_(false)
  {
  }
  // Allocate storage for 'size' elements of type 'T',
  // initially placed in location 'where'.
  storage_manager(allocator *a, length_type size, location where);
  ~storage_manager()
  {
  }

  void rebind(T *ptr, length_type size)
  {
    OVXX_PRECONDITION(!admitted());
    user_storage_.rebind(ptr, size);
    use_user_storage_ = t::is_compatible(user_storage_.format());
    host_storage_.resize(size,
			 use_user_storage_ ?
			 user_storage_.template as<F>() :
			 ptr_type());
#if OVXX_HAVE_OPENCL
    opencl_storage_.resize(size);
#endif
  }
  void rebind(uT *ptr, length_type size)
  {
    OVXX_PRECONDITION(!admitted());
    user_storage_.rebind(ptr, size);
    use_user_storage_ = t::is_compatible(user_storage_.format());
    host_storage_.resize(size,
			 use_user_storage_ ?
			 user_storage_.template as<F>() :
			 ptr_type());
#if OVXX_HAVE_OPENCL
    opencl_storage_.resize(size);
#endif
  }
  void rebind(std::pair<uT*,uT*> ptr, length_type size)
  {
    OVXX_PRECONDITION(!admitted());
    user_storage_.rebind(ptr, size);
    use_user_storage_ = t::is_compatible(user_storage_.format());
    host_storage_.resize(size,
			 use_user_storage_ ?
			 user_storage_.template as<F>() :
			 ptr_type());
#if OVXX_HAVE_OPENCL
    opencl_storage_.resize(size);
#endif
  }
  void find(T *&ptr)
  {
    OVXX_PRECONDITION(user_storage_.format() == no_user_format ||
		      user_storage_.format() == array_format);
    user_storage_.find(ptr);
  }
  void find(uT *&ptr)
  {
    OVXX_PRECONDITION(user_storage_.format() == no_user_format ||
		      user_storage_.format() == interleaved_format);
    user_storage_.find(ptr);
  }
  void find(std::pair<uT*,uT*> &ptr)
  {
    OVXX_PRECONDITION(user_storage_.format() == no_user_format ||
		      user_storage_.format() == split_format);
    user_storage_.find(ptr);
  }
  void admit(bool update=true)
  {
    if (!admitted_ && !use_user_storage_ && update)
      for (index_type i = 0; i < host_storage_.size(); ++i)
	host_storage_.put(i, user_storage_.get(i));
    admitted_ = true;
  }
  void release(bool update = true)
  {
    if (!admitted_) return;
    if (!use_user_storage_ && update)
      for (index_type i = 0; i < host_storage_.size(); ++i)
	user_storage_.put(i, host_storage_.get(i));
    admitted_ = false;
  }
  void release(bool update, T*&ptr)
  {
    OVXX_PRECONDITION(user_storage_.format() == no_user_format ||
		      user_storage_.format() == array_format);
    release(update);
    user_storage_.find(ptr);
  }
  void release(bool update, uT*&ptr)
  {
    OVXX_PRECONDITION(user_storage_.format() == no_user_format ||
		      user_storage_.format() == interleaved_format);
    release(update);
    user_storage_.find(ptr);
  }
  void release(bool update, std::pair<uT*,uT*> &ptr)
  {
    OVXX_PRECONDITION(user_storage_.format() == no_user_format ||
		      user_storage_.format() == split_format);
    release(update);
    user_storage_.find(ptr);
  }
  user_storage_type user_storage() const { return user_storage_.format();}
  bool admitted() const { return admitted_;}
  // Modify the data in the given location, invalidate all replica.
  // Get a valid writable pointer in the given address space.
  ptr_type ptr(location where = 0)
  {
    //    if (where == 0)
    {
      sync(0);
      invalidate(~0);
      return host_storage_.ptr();
    }
// #if OVXX_HAVE_OPENCL
//     else // device
//     {
//       sync(~0);
//       invalidate(0);
//       return opencl_storage_.ptr();
//     }
// #endif
  }
  // get a valid read-only pointer in the given address space
  const_ptr_type ptr(location where = 0) const
  {
    // if (where == 0)
    {
      sync(0);
      return host_storage_.ptr();
    }
// #if OVXX_HAVE_OPENCL
//     else
//     {
//       sync(~0);
//       return opencl_storage_.ptr();
//     }
// #endif
  }
#if OVXX_HAVE_OPENCL
  opencl::buffer buffer()
  {
    sync(1); // TODO: define proper location for OpenCL storage
    invalidate(0);
    return opencl_storage_.ptr();
  }
  opencl::buffer buffer() const
  {
    sync(1); // TODO: define proper location for OpenCL storage
    invalidate(0);
    return opencl_storage_.ptr();
  }
#endif

  value_type get(index_type i) const
  {
    sync(0);
    return host_storage_.get(i);
  }
  void put(index_type i, value_type v)
  {
    sync(0);
    host_storage_.put(i, v);
    invalidate(~0);
  }
  reference_type at(index_type i)
  {
    sync(0);
    return host_storage_.at(i);
  }
  const_reference_type at(index_type i) const
  {
    sync(0);
    return host_storage_.at(i);
  }

private:
  // Make sure storage is available in the given location,
  // and data is valid.
  void sync(location where) const
  {
#if OVXX_HAVE_OPENCL
    if (where == 0) // host
    {
      // 1. allocate if necessary
      host_storage_.allocate();
      // 2. copy data to host, if necessary
      //    and mark host as valid
      if (!host_storage_.is_valid())
      {
	opencl_storage_.copy_to_host(host_storage_.ptr());
	host_storage_.validate();
      }
    }
    else // device
    {
      // 1. allocate if necessary
      opencl_storage_.allocate();
      // 2. copy data from host, if necessary
      //    and mark device as valid
      if (!opencl_storage_.is_valid())
      {
	opencl_storage_.copy_from_host(host_storage_.ptr());
	opencl_storage_.validate();
      }
    }
#else
    host_storage_.allocate();
#endif
  }
  void invalidate(location where) const
  {
#if OVXX_HAVE_OPENCL
    if (where == 0) // host
    {
      // 1. mark host as invalid
      host_storage_.invalidate();
    }
    else // device
    {
      // 1. mark opencl device as invalid
      opencl_storage_.invalidate();
    }
#endif    
  }

  ovxx::user_storage<T> user_storage_;
  bool use_user_storage_;
  mutable host_storage<T, F> host_storage_;
#if OVXX_HAVE_OPENCL
  mutable opencl::storage<T, F> opencl_storage_;
#endif
  bool admitted_;
};

} // namespace ovxx

#endif
