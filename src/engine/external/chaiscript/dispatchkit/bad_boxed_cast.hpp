 
 
 
 
 

 
 

#ifndef CHAISCRIPT_BAD_BOXED_CAST_HPP_
#define CHAISCRIPT_BAD_BOXED_CAST_HPP_

#include <string>
#include <typeinfo>

#include "../chaiscript_defines.hpp"
#include "../utility/static_string.hpp"
#include "type_info.hpp"

namespace chaiscript {
  class Type_Info;
}  

namespace chaiscript {
  namespace exception {
     
     
     
     
     
    class bad_boxed_cast : public std::bad_cast {
    public:
      bad_boxed_cast(Type_Info t_from, const std::type_info &t_to, utility::Static_String t_what) noexcept
          : from(t_from)
          , to(&t_to)
          , m_what(std::move(t_what)) {
      }

      bad_boxed_cast(Type_Info t_from, const std::type_info &t_to) noexcept
          : from(t_from)
          , to(&t_to)
          , m_what("Cannot perform boxed_cast") {
      }

      explicit bad_boxed_cast(utility::Static_String t_what) noexcept
          : m_what(std::move(t_what)) {
      }

      bad_boxed_cast(const bad_boxed_cast &) noexcept = default;
      ~bad_boxed_cast() noexcept override = default;

       
      const char *what() const noexcept override { return m_what.c_str(); }

      Type_Info from;  
      const std::type_info *to = nullptr;  

    private:
      utility::Static_String m_what;
    };
  }  
}  

#endif
