 
 
 
 
 

#ifndef CHAISCRIPT_UNKNOWN_HPP_
#define CHAISCRIPT_UNKNOWN_HPP_

namespace chaiscript {
  namespace detail {
    struct Loadable_Module {
      Loadable_Module(const std::string &, const std::string &) {
#ifdef CHAISCRIPT_NO_DYNLOAD
        throw chaiscript::exception::load_module_error("Loadable module support was disabled (CHAISCRIPT_NO_DYNLOAD)");
#else
        throw chaiscript::exception::load_module_error("Loadable module support not available for your platform");
#endif
      }

      ModulePtr m_moduleptr;
    };
  }  
}  
#endif
