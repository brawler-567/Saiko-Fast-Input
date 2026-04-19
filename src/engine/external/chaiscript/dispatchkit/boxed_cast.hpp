 
 
 
 
 

 
 

#ifndef CHAISCRIPT_BOXED_CAST_HPP_
#define CHAISCRIPT_BOXED_CAST_HPP_

#include "../chaiscript_defines.hpp"
#include "bad_boxed_cast.hpp"
#include "boxed_cast_helper.hpp"
#include "boxed_value.hpp"
#include "type_conversions.hpp"
#include "type_info.hpp"

namespace chaiscript {
  class Type_Conversions;
}
namespace chaiscript::detail::exception {
  class bad_any_cast;
}  

namespace chaiscript {
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  template<typename Type>
  decltype(auto) boxed_cast(const Boxed_Value &bv, const Type_Conversions_State *t_conversions = nullptr) {
    if (!t_conversions || bv.get_type_info().bare_equal(user_type<Type>()) || (t_conversions && !(*t_conversions)->convertable_type<Type>())) {
      try {
        return detail::Cast_Helper<Type>::cast(bv, t_conversions);
      } catch (const chaiscript::detail::exception::bad_any_cast &) {
      }
    }

    if (t_conversions && (*t_conversions)->convertable_type<Type>()) {
      try {
         
         
        return (detail::Cast_Helper<Type>::cast((*t_conversions)->boxed_type_conversion<Type>(t_conversions->saves(), bv), t_conversions));
      } catch (...) {
        try {
           
          return (detail::Cast_Helper<Type>::cast((*t_conversions)->boxed_type_down_conversion<Type>(t_conversions->saves(), bv),
                                                  t_conversions));
        } catch (const chaiscript::detail::exception::bad_any_cast &) {
          throw exception::bad_boxed_cast(bv.get_type_info(), typeid(Type));
        }
      }
    } else {
       
       
      throw exception::bad_boxed_cast(bv.get_type_info(), typeid(Type));
    }
  }

}  

#endif
