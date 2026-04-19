 
 
 
 
 

 
 

#ifndef CHAISCRIPT_ENGINE_HPP_
#define CHAISCRIPT_ENGINE_HPP_

#include <cassert>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <vector>

#include "../chaiscript_defines.hpp"
#include "../chaiscript_threading.hpp"
#include "../dispatchkit/boxed_cast_helper.hpp"
#include "../dispatchkit/boxed_value.hpp"
#include "../dispatchkit/dispatchkit.hpp"
#include "../dispatchkit/proxy_functions.hpp"
#include "../dispatchkit/register_function.hpp"
#include "../dispatchkit/type_conversions.hpp"
#include "chaiscript_common.hpp"

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__HAIKU__)
#include <unistd.h>
#endif

#if !defined(CHAISCRIPT_NO_DYNLOAD) && defined(_POSIX_VERSION) && !defined(__CYGWIN__)
#include <dlfcn.h>
#endif

#if defined(CHAISCRIPT_NO_DYNLOAD)
#include "chaiscript_unknown.hpp"
#elif defined(CHAISCRIPT_WINDOWS)
#include "chaiscript_windows.hpp"
#elif _POSIX_VERSION
#include "chaiscript_posix.hpp"
#else
#include "chaiscript_unknown.hpp"
#endif

#include "../dispatchkit/exception_specification.hpp"

namespace chaiscript {
   
  using Namespace = dispatch::Dynamic_Object;

  namespace detail {
    using Loadable_Module_Ptr = std::shared_ptr<Loadable_Module>;
  }

   
  class ChaiScript_Basic {
    mutable chaiscript::detail::threading::shared_mutex m_mutex;
    mutable chaiscript::detail::threading::recursive_mutex m_use_mutex;

    std::set<std::string> m_used_files;
    std::map<std::string, detail::Loadable_Module_Ptr> m_loaded_modules;
    std::set<std::string> m_active_loaded_modules;

    std::vector<std::string> m_module_paths;
    std::vector<std::string> m_use_paths;

    std::unique_ptr<parser::ChaiScript_Parser_Base> m_parser;

    chaiscript::detail::Dispatch_Engine m_engine;

    std::map<std::string, std::function<Namespace &()>> m_namespace_generators;

     
    Boxed_Value do_eval(const std::string &t_input, const std::string &t_filename = "__EVAL__", bool   = false) {
      try {
        const auto p = m_parser->parse(t_input, t_filename);
        return p->eval(chaiscript::detail::Dispatch_State(m_engine));
      } catch (chaiscript::eval::detail::Return_Value &rv) {
        return rv.retval;
      }
    }

     
    Boxed_Value internal_eval_file(const std::string &t_filename) {
      for (const auto &path : m_use_paths) {
        try {
          const auto appendedpath = path + t_filename;
          return do_eval(load_file(appendedpath), appendedpath, true);
        } catch (const exception::file_not_found_error &) {
           
        } catch (const exception::eval_error &t_ee) {
          throw Boxed_Value(t_ee);
        }
      }

       
      throw exception::file_not_found_error(t_filename);
    }

     
    Boxed_Value internal_eval(const std::string &t_e) {
      try {
        return do_eval(t_e, "__EVAL__", true);
      } catch (const exception::eval_error &t_ee) {
        throw Boxed_Value(t_ee);
      }
    }

     
    chaiscript::detail::Dispatch_Engine &get_eval_engine() noexcept { return m_engine; }

     
    void build_eval_system(const ModulePtr &t_lib, const std::vector<Options> &t_opts) {
      if (t_lib) {
        add(t_lib);
      }

      m_engine.add(fun([this]() { m_engine.dump_system(); }), "dump_system");
      m_engine.add(fun([this](const Boxed_Value &t_bv) { m_engine.dump_object(t_bv); }), "dump_object");
      m_engine.add(fun([this](const Boxed_Value &t_bv, const std::string &t_type) { return m_engine.is_type(t_bv, t_type); }), "is_type");
      m_engine.add(fun([this](const Boxed_Value &t_bv) { return m_engine.type_name(t_bv); }), "type_name");
      m_engine.add(fun([this](const std::string &t_f) { return m_engine.function_exists(t_f); }), "function_exists");
      m_engine.add(fun([this]() { return m_engine.get_function_objects(); }), "get_functions");
      m_engine.add(fun([this]() { return m_engine.get_scripting_objects(); }), "get_objects");

      m_engine.add(dispatch::make_dynamic_proxy_function([this](const Function_Params &t_params) { return m_engine.call_exists(t_params); }),
                   "call_exists");

      m_engine.add(fun([this](const dispatch::Proxy_Function_Base &t_fun, const std::vector<Boxed_Value> &t_params) -> Boxed_Value {
                     Type_Conversions_State s(this->m_engine.conversions(), this->m_engine.conversions().conversion_saves());
                     return t_fun(Function_Params{t_params}, s);
                   }),
                   "call");

      m_engine.add(fun([this](const Type_Info &t_ti) { return m_engine.get_type_name(t_ti); }), "name");

      m_engine.add(fun([this](const std::string &t_type_name, bool t_throw) { return m_engine.get_type(t_type_name, t_throw); }), "type");
      m_engine.add(fun([this](const std::string &t_type_name) { return m_engine.get_type(t_type_name, true); }), "type");

      m_engine.add(fun([this](const Type_Info &t_from, const Type_Info &t_to, const std::function<Boxed_Value(const Boxed_Value &)> &t_func) {
                     m_engine.add(chaiscript::type_conversion(t_from, t_to, t_func));
                   }),
                   "add_type_conversion");

      if (std::find(t_opts.begin(), t_opts.end(), Options::No_Load_Modules) == t_opts.end()
          && std::find(t_opts.begin(), t_opts.end(), Options::Load_Modules) != t_opts.end()) {
        m_engine.add(fun([this](const std::string &t_module, const std::string &t_file) { load_module(t_module, t_file); }), "load_module");
        m_engine.add(fun([this](const std::string &t_module) { return load_module(t_module); }), "load_module");
      }

      if (std::find(t_opts.begin(), t_opts.end(), Options::No_External_Scripts) == t_opts.end()
          && std::find(t_opts.begin(), t_opts.end(), Options::External_Scripts) != t_opts.end()) {
        m_engine.add(fun([this](const std::string &t_file) { return use(t_file); }), "use");
        m_engine.add(fun([this](const std::string &t_file) { return internal_eval_file(t_file); }), "eval_file");
      }

      m_engine.add(fun([this](const std::string &t_str) { return internal_eval(t_str); }), "eval");
      m_engine.add(fun([this](const AST_Node &t_ast) { return eval(t_ast); }), "eval");

      m_engine.add(fun([this](const std::string &t_str, const bool t_dump) { return parse(t_str, t_dump); }), "parse");
      m_engine.add(fun([this](const std::string &t_str) { return parse(t_str); }), "parse");

      m_engine.add(fun([this](const Boxed_Value &t_bv, const std::string &t_name) { add_global_const(t_bv, t_name); }), "add_global_const");
      m_engine.add(fun([this](const Boxed_Value &t_bv, const std::string &t_name) { add_global(t_bv, t_name); }), "add_global");
      m_engine.add(fun([this](const Boxed_Value &t_bv, const std::string &t_name) { set_global(t_bv, t_name); }), "set_global");

       
      m_engine.add(fun([this](const std::string &t_namespace_name) {
                     register_namespace([](Namespace &  ) noexcept {}, t_namespace_name);
                     import(t_namespace_name);
                   }),
                   "namespace");
      m_engine.add(fun([this](const std::string &t_namespace_name) { import(t_namespace_name); }), "import");
    }

     
    static bool skip_bom(std::ifstream &infile) {
      size_t bytes_needed = 3;
      char buffer[3];

      memset(buffer, '\0', bytes_needed);

      infile.read(buffer, static_cast<std::streamsize>(bytes_needed));

      if ((buffer[0] == '\xef') && (buffer[1] == '\xbb') && (buffer[2] == '\xbf')) {
        infile.seekg(3);
        return true;
      }

      infile.seekg(0);

      return false;
    }

     
    static std::string load_file(const std::string &t_filename) {
      std::ifstream infile(t_filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary);

      if (!infile.is_open()) {
        throw chaiscript::exception::file_not_found_error(t_filename);
      }

      auto size = infile.tellg();
      infile.seekg(0, std::ios::beg);

      assert(size >= 0);

      if (skip_bom(infile)) {
        size -= 3;  
        assert(size >= 0);  
      }

      if (size == std::streampos(0)) {
        return std::string();
      } else {
        std::vector<char> v(static_cast<size_t>(size));
        infile.read(&v[0], static_cast<std::streamsize>(size));
        return std::string(v.begin(), v.end());
      }
    }

    std::vector<std::string> ensure_minimum_path_vec(std::vector<std::string> paths) {
      if (paths.empty()) {
        return {""};
      } else {
        return paths;
      }
    }

  public:
     
     
    virtual ~ChaiScript_Basic() = default;
     
     
     
     
     
    ChaiScript_Basic(const ModulePtr &t_lib,
                     std::unique_ptr<parser::ChaiScript_Parser_Base> &&parser,
                     std::vector<std::string> t_module_paths = {},
                     std::vector<std::string> t_use_paths = {},
                     const std::vector<chaiscript::Options> &t_opts = chaiscript::default_options())
        : m_module_paths(ensure_minimum_path_vec(std::move(t_module_paths)))
        , m_use_paths(ensure_minimum_path_vec(std::move(t_use_paths)))
        , m_parser(std::move(parser))
        , m_engine(*m_parser) {
#if !defined(CHAISCRIPT_NO_DYNLOAD) && defined(_POSIX_VERSION) && !defined(__CYGWIN__)
       
       

      union cast_union {
        Boxed_Value (ChaiScript_Basic::*in_ptr)(const std::string &);
        void *out_ptr;
      };

      Dl_info rInfo;
      memset(&rInfo, 0, sizeof(rInfo));
      cast_union u;
      u.in_ptr = &ChaiScript_Basic::use;
      if ((dladdr(static_cast<void *>(u.out_ptr), &rInfo) != 0) && (rInfo.dli_fname != nullptr)) {
        std::string dllpath(rInfo.dli_fname);
        const size_t lastslash = dllpath.rfind('/');
        if (lastslash != std::string::npos) {
          dllpath.erase(lastslash);
        }

         
        std::vector<char> buf(2048);
        const auto pathlen = readlink(dllpath.c_str(), &buf.front(), buf.size());
        if (pathlen > 0 && static_cast<size_t>(pathlen) < buf.size()) {
          dllpath = std::string(&buf.front(), static_cast<size_t>(pathlen));
        }

        m_module_paths.insert(m_module_paths.begin(), dllpath + "/");
      }
#endif
      build_eval_system(t_lib, t_opts);
    }

#ifndef CHAISCRIPT_NO_DYNLOAD
     
     
     
     
     
     
     
    explicit ChaiScript_Basic(std::unique_ptr<parser::ChaiScript_Parser_Base> &&parser,
                              std::vector<std::string> t_module_paths = {},
                              std::vector<std::string> t_use_paths = {},
                              const std::vector<chaiscript::Options> &t_opts = chaiscript::default_options())
        : ChaiScript_Basic({}, std::move(parser), t_module_paths, t_use_paths, t_opts) {
      try {
         
        load_module("chaiscript_stdlib-" + Build_Info::version());
      } catch (const exception::load_module_error &t_err) {
        std::cout << "An error occurred while trying to load the chaiscript standard library.\n"
                     "\n"
                     "You must either provide a standard library, or compile it in.\n"
                     "For an example of compiling the standard library in,\n"
                     "see: https://gist.github.com/lefticus/9456197\n"
                     "Compiling the stdlib in is the recommended and MOST SUPPORTED method.\n"
                     "\n\n"
                  << t_err.what();
        throw;
      }
    }
#else  
    explicit ChaiScript_Basic(std::unique_ptr<parser::ChaiScript_Parser_Base> &&parser,
                              std::vector<std::string> t_module_paths = {},
                              std::vector<std::string> t_use_paths = {},
                              const std::vector<chaiscript::Options> &t_opts = chaiscript::default_options())
        = delete;
#endif

    parser::ChaiScript_Parser_Base &get_parser() noexcept {
      return *m_parser;
    }

    const Boxed_Value eval(const AST_Node &t_ast) {
      try {
        return t_ast.eval(chaiscript::detail::Dispatch_State(m_engine));
      } catch (const exception::eval_error &t_ee) {
        throw Boxed_Value(t_ee);
      }
    }

    AST_NodePtr parse(const std::string &t_input, const bool t_debug_print = false) {
      auto ast = m_parser->parse(t_input, "PARSE");
      if (t_debug_print) {
        m_parser->debug_print(*ast);
      }
      return ast;
    }

    std::string get_type_name(const Type_Info &ti) const { return m_engine.get_type_name(ti); }

    template<typename T>
    std::string get_type_name() const {
      return get_type_name(user_type<T>());
    }

     
     
     
     
     
    Boxed_Value use(const std::string &t_filename) {
      for (const auto &path : m_use_paths) {
        const auto appendedpath = path + t_filename;
        try {
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l2(m_mutex);

          Boxed_Value retval;

          if (m_used_files.count(appendedpath) == 0) {
            l2.unlock();
            retval = eval_file(appendedpath);
            l2.lock();
            m_used_files.insert(appendedpath);
          }

          return retval;  
        } catch (const exception::file_not_found_error &e) {
          if (e.filename != appendedpath) {
             
            throw;
          }
           
        }
      }

       
      throw exception::file_not_found_error(t_filename);
    }

     
     
     
     
     
    ChaiScript_Basic &add_global_const(const Boxed_Value &t_bv, const std::string &t_name) {
      Name_Validator::validate_object_name(t_name);
      m_engine.add_global_const(t_bv, t_name);
      return *this;
    }

     
     
     
     
     
    ChaiScript_Basic &add_global(const Boxed_Value &t_bv, const std::string &t_name) {
      Name_Validator::validate_object_name(t_name);
      m_engine.add_global(t_bv, t_name);
      return *this;
    }

    ChaiScript_Basic &set_global(const Boxed_Value &t_bv, const std::string &t_name) {
      Name_Validator::validate_object_name(t_name);
      m_engine.set_global(t_bv, t_name);
      return *this;
    }

     
     
     
     
     
    struct State {
      std::set<std::string> used_files;
      chaiscript::detail::Dispatch_Engine::State engine_state;
      std::set<std::string> active_loaded_modules;
    };

     
     
     
     
     
     
     
     
     
     
     
     
     
    State get_state() const {
      chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
      chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l2(m_mutex);

      State s;
      s.used_files = m_used_files;
      s.engine_state = m_engine.get_state();
      s.active_loaded_modules = m_active_loaded_modules;
      return s;
    }

     
     
     
     
     
     
     
     
     
     
     
     
     
     
    void set_state(const State &t_state) {
      chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
      chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l2(m_mutex);

      m_used_files = t_state.used_files;
      m_active_loaded_modules = t_state.active_loaded_modules;
      m_engine.set_state(t_state.engine_state);
    }

     
    std::map<std::string, Boxed_Value> get_locals() const { return m_engine.get_locals(); }

     
     
     
     
     
    void set_locals(const std::map<std::string, Boxed_Value> &t_locals) { m_engine.set_locals(t_locals); }

     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
    template<typename T>
    ChaiScript_Basic &add(const T &t_t, const std::string &t_name) {
      Name_Validator::validate_object_name(t_name);
      m_engine.add(t_t, t_name);
      return *this;
    }

     
     
     
     
     
     
     
     
     
    ChaiScript_Basic &add(const Type_Conversion &d) {
      m_engine.add(d);
      return *this;
    }

     
     
     
    ChaiScript_Basic &add(const ModulePtr &t_p) {
      t_p->apply(*this, this->get_eval_engine());
      return *this;
    }

     
     
     
     
     
     
     
     
     
     
     
     
    std::string load_module(const std::string &t_module_name) {
#ifdef CHAISCRIPT_NO_DYNLOAD
      throw chaiscript::exception::load_module_error("Loadable module support was disabled (CHAISCRIPT_NO_DYNLOAD)");
#else
      std::vector<exception::load_module_error> errors;
      std::string version_stripped_name = t_module_name;
      size_t version_pos = version_stripped_name.find("-" + Build_Info::version());
      if (version_pos != std::string::npos) {
        version_stripped_name.erase(version_pos);
      }

      std::vector<std::string> prefixes{"lib", "cyg", ""};

      std::vector<std::string> postfixes{".dll", ".so", ".bundle", ""};

      for (auto &elem : m_module_paths) {
        for (auto &prefix : prefixes) {
          for (auto &postfix : postfixes) {
            try {
              const auto name = elem + prefix + t_module_name + postfix;
               
              load_module(version_stripped_name, name);
              return name;
            } catch (const chaiscript::exception::load_module_error &e) {
               
              errors.push_back(e);
               
            }
          }
        }
      }

      throw chaiscript::exception::load_module_error(t_module_name, errors);
#endif
    }

     
     
     
     
     
     
     
    void load_module(const std::string &t_module_name, const std::string &t_filename) {
      chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);

      if (m_loaded_modules.count(t_module_name) == 0) {
        detail::Loadable_Module_Ptr lm(new detail::Loadable_Module(t_module_name, t_filename));
        m_loaded_modules[t_module_name] = lm;
        m_active_loaded_modules.insert(t_module_name);
        add(lm->m_moduleptr);
      } else if (m_active_loaded_modules.count(t_module_name) == 0) {
        m_active_loaded_modules.insert(t_module_name);
        add(m_loaded_modules[t_module_name]->m_moduleptr);
      }
    }

     
     
     
     
     
     
     
     
    Boxed_Value operator()(const std::string &t_script, const Exception_Handler &t_handler = Exception_Handler()) {
      return eval(t_script, t_handler);
    }

     
     
     
     
     
     
     
     
     
     
     
     
     
    template<typename T>
    T eval(const std::string &t_input, const Exception_Handler &t_handler = Exception_Handler(), const std::string &t_filename = "__EVAL__") {
      return m_engine.boxed_cast<T>(eval(t_input, t_handler, t_filename));
    }

     
    template<typename Type>
    decltype(auto) boxed_cast(const Boxed_Value &bv) const {
      return (m_engine.boxed_cast<Type>(bv));
    }

     
     
     
     
     
     
     
     
     
     
    Boxed_Value
    eval(const std::string &t_input, const Exception_Handler &t_handler = Exception_Handler(), const std::string &t_filename = "__EVAL__") {
      try {
        return do_eval(t_input, t_filename);
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv, m_engine);
        }
        throw;
      }
    }

     
     
     
     
     
    Boxed_Value eval_file(const std::string &t_filename, const Exception_Handler &t_handler = Exception_Handler()) {
      return eval(load_file(t_filename), t_handler, t_filename);
    }

     
     
     
     
     
     
     
     
    template<typename T>
    T eval_file(const std::string &t_filename, const Exception_Handler &t_handler = Exception_Handler()) {
      return m_engine.boxed_cast<T>(eval_file(t_filename, t_handler));
    }

     
     
     
    void import(const std::string &t_namespace_name) {
      chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);

      if (m_engine.get_scripting_objects().count(t_namespace_name)) {
        throw std::runtime_error("Namespace: " + t_namespace_name + " was already defined");
      } else if (m_namespace_generators.count(t_namespace_name)) {
        m_engine.add_global(var(std::ref(m_namespace_generators[t_namespace_name]())), t_namespace_name);
      } else {
        throw std::runtime_error("No registered namespace: " + t_namespace_name);
      }
    }

     
     
     
    void register_namespace(const std::function<void(Namespace &)> &t_namespace_generator, const std::string &t_namespace_name) {
      chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);

      if (!m_namespace_generators.count(t_namespace_name)) {
         
        m_namespace_generators.emplace(std::make_pair(t_namespace_name, [=, space = Namespace()]() mutable -> Namespace & {
          t_namespace_generator(space);
          return space;
        }));
      } else {
        throw std::runtime_error("Namespace: " + t_namespace_name + " was already registered.");
      }
    }
  };

}  
#endif  
