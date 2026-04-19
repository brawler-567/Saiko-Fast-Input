 
 
 
 
 

 
 

#ifndef CHAISCRIPT_FUNCTION_CALL_HPP_
#define CHAISCRIPT_FUNCTION_CALL_HPP_

#include <functional>
#include <string>
#include <vector>

#include "boxed_cast.hpp"
#include "function_call_detail.hpp"
#include "proxy_functions.hpp"

namespace chaiscript {
  class Boxed_Value;
  class Type_Conversions_State;
  namespace detail {
    template<typename T>
    struct Cast_Helper;
  }  
}  

namespace chaiscript {
  namespace dispatch {
    namespace detail {
      template<typename Ret, typename... Param>
      constexpr auto arity(Ret (*)(Param...)) noexcept {
        return sizeof...(Param);
      }
    }  

     
     
     
     
     
     
    template<typename FunctionType>
    std::function<FunctionType> functor(const std::vector<Const_Proxy_Function> &funcs, const Type_Conversions_State *t_conversions) {
      const bool has_arity_match = std::any_of(funcs.begin(), funcs.end(), [](const Const_Proxy_Function &f) {
        return f->get_arity() == -1 || size_t(f->get_arity()) == detail::arity(static_cast<FunctionType *>(nullptr));
      });

      if (!has_arity_match) {
        throw exception::bad_boxed_cast(user_type<Const_Proxy_Function>(), typeid(std::function<FunctionType>));
      }

      FunctionType *p = nullptr;
      return detail::build_function_caller_helper(p, funcs, t_conversions);
    }

     
     
     
     
     
     
     
     
     
     
     
    template<typename FunctionType>
    std::function<FunctionType> functor(Const_Proxy_Function func, const Type_Conversions_State *t_conversions) {
      return functor<FunctionType>(std::vector<Const_Proxy_Function>({std::move(func)}), t_conversions);
    }

     
     
    template<typename FunctionType>
    std::function<FunctionType> functor(const Boxed_Value &bv, const Type_Conversions_State *t_conversions) {
      return functor<FunctionType>(boxed_cast<Const_Proxy_Function>(bv, t_conversions), t_conversions);
    }
  }  

  namespace detail {
     
    template<typename Signature>
    struct Cast_Helper<const std::function<Signature> &> {
      static std::function<Signature> cast(const Boxed_Value &ob, const Type_Conversions_State *t_conversions) {
        if (ob.get_type_info().bare_equal(user_type<Const_Proxy_Function>())) {
          return dispatch::functor<Signature>(ob, t_conversions);
        } else {
          return Cast_Helper_Inner<const std::function<Signature> &>::cast(ob, t_conversions);
        }
      }
    };

     
    template<typename Signature>
    struct Cast_Helper<std::function<Signature>> {
      static std::function<Signature> cast(const Boxed_Value &ob, const Type_Conversions_State *t_conversions) {
        if (ob.get_type_info().bare_equal(user_type<Const_Proxy_Function>())) {
          return dispatch::functor<Signature>(ob, t_conversions);
        } else {
          return Cast_Helper_Inner<std::function<Signature>>::cast(ob, t_conversions);
        }
      }
    };

     
    template<typename Signature>
    struct Cast_Helper<const std::function<Signature>> {
      static std::function<Signature> cast(const Boxed_Value &ob, const Type_Conversions_State *t_conversions) {
        if (ob.get_type_info().bare_equal(user_type<Const_Proxy_Function>())) {
          return dispatch::functor<Signature>(ob, t_conversions);
        } else {
          return Cast_Helper_Inner<const std::function<Signature>>::cast(ob, t_conversions);
        }
      }
    };
  }  
}  

#endif
