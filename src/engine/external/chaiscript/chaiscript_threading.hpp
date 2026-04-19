 
 
 
 
 

 
 

#ifndef CHAISCRIPT_THREADING_HPP_
#define CHAISCRIPT_THREADING_HPP_

#include <unordered_map>

#ifndef CHAISCRIPT_NO_THREADS
#include <mutex>
#include <shared_mutex>
#include <thread>
#else
#ifndef CHAISCRIPT_NO_THREADS_WARNING
#pragma message("ChaiScript is compiling without thread safety.")
#endif
#endif

#include "chaiscript_defines.hpp"

 
 
 
 
 
 
 

 
 
 
namespace chaiscript::detail::threading {
#ifndef CHAISCRIPT_NO_THREADS

  template<typename T>
  using unique_lock = std::unique_lock<T>;

  template<typename T>
  using shared_lock = std::shared_lock<T>;

  template<typename T>
  using lock_guard = std::lock_guard<T>;

  using std::shared_mutex;

  using std::mutex;

  using std::recursive_mutex;

   
   
  template<typename T>
  class Thread_Storage {
  public:
    Thread_Storage() = default;
    Thread_Storage(const Thread_Storage &) = delete;
    Thread_Storage(Thread_Storage &&) = delete;
    Thread_Storage &operator=(const Thread_Storage &) = delete;
    Thread_Storage &operator=(Thread_Storage &&) = delete;

    ~Thread_Storage() { t().erase(this); }

    inline const T *operator->() const noexcept { return &(t()[this]); }

    inline const T &operator*() const noexcept { return t()[this]; }

    inline T *operator->() noexcept { return &(t()[this]); }

    inline T &operator*() noexcept { return t()[this]; }

    void *m_key;

  private:
     
     
    static std::unordered_map<const void *, T> &t() noexcept {
      static thread_local std::unordered_map<const void *, T> my_t;
      return my_t;
    }
  };

#else  
  template<typename T>
  class unique_lock {
  public:
    constexpr explicit unique_lock(T &) noexcept {}
    constexpr void lock() noexcept {}
    constexpr void unlock() noexcept {}
  };

  template<typename T>
  class shared_lock {
  public:
    constexpr explicit shared_lock(T &) noexcept {}
    constexpr void lock() noexcept {}
    constexpr void unlock() noexcept {}
  };

  template<typename T>
  class lock_guard {
  public:
    constexpr explicit lock_guard(T &) noexcept {}
  };

  class shared_mutex {
  };

  class recursive_mutex {
  };

  template<typename T>
  class Thread_Storage {
  public:
    constexpr explicit Thread_Storage() noexcept {}

    constexpr inline T *operator->() const noexcept { return &obj; }

    constexpr inline T &operator*() const noexcept { return obj; }

  private:
    mutable T obj;
  };

#endif
}  

#endif
