 
 
 
 
 

 
 

#ifndef CHAISCRIPT_BOXED_VALUE_HPP_
#define CHAISCRIPT_BOXED_VALUE_HPP_

#include <map>
#include <memory>
#include <type_traits>

#include "../chaiscript_defines.hpp"
#include "any.hpp"
#include "type_info.hpp"

namespace chaiscript {
   
   
  class Boxed_Value {
  public:
     
    struct Void_Type {
    };

  private:
     
     
    struct Data {
      Data(const Type_Info &ti, chaiscript::detail::Any to, bool is_ref, const void *t_void_ptr, bool t_return_value) noexcept
          : m_type_info(ti)
          , m_obj(std::move(to))
          , m_data_ptr(ti.is_const() ? nullptr : const_cast<void *>(t_void_ptr))
          , m_const_data_ptr(t_void_ptr)
          , m_is_ref(is_ref)
          , m_return_value(t_return_value) {
      }

      Data &operator=(const Data &rhs) {
        m_type_info = rhs.m_type_info;
        m_obj = rhs.m_obj;
        m_is_ref = rhs.m_is_ref;
        m_data_ptr = rhs.m_data_ptr;
        m_const_data_ptr = rhs.m_const_data_ptr;
        m_return_value = rhs.m_return_value;

        if (rhs.m_attrs) {
          m_attrs = std::make_unique<std::map<std::string, std::shared_ptr<Data>>>(*rhs.m_attrs);
        }

        return *this;
      }

      Data(const Data &) = delete;

      Data(Data &&) = default;
      Data &operator=(Data &&rhs) = default;

      Type_Info m_type_info;
      chaiscript::detail::Any m_obj;
      void *m_data_ptr;
      const void *m_const_data_ptr;
      std::unique_ptr<std::map<std::string, std::shared_ptr<Data>>> m_attrs;
      bool m_is_ref;
      bool m_return_value;
    };

    struct Object_Data {
      static auto get(Boxed_Value::Void_Type, bool t_return_value) {
        return std::make_shared<Data>(detail::Get_Type_Info<void>::get(), chaiscript::detail::Any(), false, nullptr, t_return_value);
      }

      template<typename T>
      static auto get(const std::shared_ptr<T> *obj, bool t_return_value) {
        return get(*obj, t_return_value);
      }

      template<typename T>
      static auto get(const std::shared_ptr<T> &obj, bool t_return_value) {
        return std::make_shared<Data>(detail::Get_Type_Info<T>::get(), chaiscript::detail::Any(obj), false, obj.get(), t_return_value);
      }

      template<typename T>
      static auto get(std::shared_ptr<T> &&obj, bool t_return_value) {
        auto ptr = obj.get();
        return std::make_shared<Data>(detail::Get_Type_Info<T>::get(), chaiscript::detail::Any(std::move(obj)), false, ptr, t_return_value);
      }

      template<typename T>
      static auto get(T *t, bool t_return_value) {
        return get(std::ref(*t), t_return_value);
      }

      template<typename T>
      static auto get(const T *t, bool t_return_value) {
        return get(std::cref(*t), t_return_value);
      }

      template<typename T>
      static auto get(std::reference_wrapper<T> obj, bool t_return_value) {
        auto p = &obj.get();
        return std::make_shared<Data>(detail::Get_Type_Info<T>::get(), chaiscript::detail::Any(std::move(obj)), true, p, t_return_value);
      }

      template<typename T>
      static auto get(std::unique_ptr<T> &&obj, bool t_return_value) {
        auto ptr = obj.get();
        return std::make_shared<Data>(detail::Get_Type_Info<T>::get(),
                                      chaiscript::detail::Any(std::make_shared<std::unique_ptr<T>>(std::move(obj))),
                                      true,
                                      ptr,
                                      t_return_value);
      }

      template<typename T>
      static auto get(T t, bool t_return_value) {
        auto p = std::make_shared<T>(std::move(t));
        auto ptr = p.get();
        return std::make_shared<Data>(detail::Get_Type_Info<T>::get(), chaiscript::detail::Any(std::move(p)), false, ptr, t_return_value);
      }

      static std::shared_ptr<Data> get() { return std::make_shared<Data>(Type_Info(), chaiscript::detail::Any(), false, nullptr, false); }
    };

  public:
     
    template<typename T, typename = std::enable_if_t<!std::is_same_v<Boxed_Value, std::decay_t<T>>>>
    explicit Boxed_Value(T &&t, bool t_return_value = false)
        : m_data(Object_Data::get(std::forward<T>(t), t_return_value)) {
    }

     
    Boxed_Value() = default;

    Boxed_Value(Boxed_Value &&) = default;
    Boxed_Value &operator=(Boxed_Value &&) = default;
    Boxed_Value(const Boxed_Value &) = default;
    Boxed_Value &operator=(const Boxed_Value &) = default;

    void swap(Boxed_Value &rhs) noexcept { std::swap(m_data, rhs.m_data); }

     
     
    Boxed_Value assign(const Boxed_Value &rhs) noexcept {
      (*m_data) = (*rhs.m_data);
      return *this;
    }

    const Type_Info &get_type_info() const noexcept { return m_data->m_type_info; }

     
    bool is_undef() const noexcept { return m_data->m_type_info.is_undef(); }

    bool is_const() const noexcept { return m_data->m_type_info.is_const(); }

    bool is_type(const Type_Info &ti) const noexcept { return m_data->m_type_info.bare_equal(ti); }

    template<typename T>
    auto pointer_sentinel(std::shared_ptr<T> &ptr) const noexcept {
      struct Sentinel {
        Sentinel(std::shared_ptr<T> &t_ptr, Data &data)
            : m_ptr(t_ptr)
            , m_data(data) {
        }

        ~Sentinel() {
           
          const auto ptr_ = m_ptr.get().get();
          m_data.get().m_data_ptr = ptr_;
          m_data.get().m_const_data_ptr = ptr_;
        }

        Sentinel &operator=(Sentinel &&s) = default;
        Sentinel(Sentinel &&s) = default;

        operator std::shared_ptr<T> &() const noexcept { return m_ptr.get(); }

        Sentinel &operator=(const Sentinel &) = delete;
        Sentinel(Sentinel &) = delete;

        std::reference_wrapper<std::shared_ptr<T>> m_ptr;
        std::reference_wrapper<Data> m_data;
      };

      return Sentinel(ptr, *(m_data.get()));
    }

    bool is_null() const noexcept { return (m_data->m_data_ptr == nullptr && m_data->m_const_data_ptr == nullptr); }

    const chaiscript::detail::Any &get() const noexcept { return m_data->m_obj; }

    bool is_ref() const noexcept { return m_data->m_is_ref; }

    bool is_return_value() const noexcept { return m_data->m_return_value; }

    void reset_return_value() const noexcept { m_data->m_return_value = false; }

    bool is_pointer() const noexcept { return !is_ref(); }

    void *get_ptr() const noexcept { return m_data->m_data_ptr; }

    const void *get_const_ptr() const noexcept { return m_data->m_const_data_ptr; }

    Boxed_Value get_attr(const std::string &t_name) {
      if (!m_data->m_attrs) {
        m_data->m_attrs = std::make_unique<std::map<std::string, std::shared_ptr<Data>>>();
      }

      auto &attr = (*m_data->m_attrs)[t_name];
      if (attr) {
        return Boxed_Value(attr, Internal_Construction());
      } else {
        Boxed_Value bv;  
        attr = bv.m_data;
        return bv;
      }
    }

    Boxed_Value &copy_attrs(const Boxed_Value &t_obj) {
      if (t_obj.m_data->m_attrs) {
        m_data->m_attrs = std::make_unique<std::map<std::string, std::shared_ptr<Data>>>(*t_obj.m_data->m_attrs);
      }
      return *this;
    }

    Boxed_Value &clone_attrs(const Boxed_Value &t_obj) {
      copy_attrs(t_obj);
      reset_return_value();
      return *this;
    }

     
    static bool type_match(const Boxed_Value &l, const Boxed_Value &r) noexcept { return l.get_type_info() == r.get_type_info(); }

  private:
     
    struct Internal_Construction {
    };

    Boxed_Value(std::shared_ptr<Data> t_data, Internal_Construction)
        : m_data(std::move(t_data)) {
    }

    std::shared_ptr<Data> m_data = Object_Data::get();
  };

   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  template<typename T>
  Boxed_Value var(T &&t) {
    return Boxed_Value(std::forward<T>(t));
  }

  namespace detail {
     
     
     
     
    template<typename T>
    Boxed_Value const_var_impl(const T &t) {
      return Boxed_Value(std::make_shared<typename std::add_const<T>::type>(t));
    }

     
     
     
     
     
    template<typename T>
    Boxed_Value const_var_impl(T *t) {
      return Boxed_Value(const_cast<typename std::add_const<T>::type *>(t));
    }

     
     
     
     
     
    template<typename T>
    Boxed_Value const_var_impl(const std::shared_ptr<T> &t) {
      return Boxed_Value(std::const_pointer_cast<typename std::add_const<T>::type>(t));
    }

     
     
     
     
     
    template<typename T>
    Boxed_Value const_var_impl(const std::reference_wrapper<T> &t) {
      return Boxed_Value(std::cref(t.get()));
    }
  }  

   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  template<typename T>
  Boxed_Value const_var(const T &t) {
    return detail::const_var_impl(t);
  }

  inline Boxed_Value void_var() {
    static const auto v = Boxed_Value(Boxed_Value::Void_Type());
    return v;
  }

  inline Boxed_Value const_var(bool b) {
    static const auto t = detail::const_var_impl(true);
    static const auto f = detail::const_var_impl(false);

    if (b) {
      return t;
    } else {
      return f;
    }
  }

}  

#endif
