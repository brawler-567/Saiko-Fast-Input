 
 

 
namespace ChaiScript_Language {
   
   
   
   
   
   
   

   
   
   
   
   
   
  class Object {
  public:
     
    Type_Info get_type_info() const;

     
    bool is_type(string) const;

     
    bool is_type(Type_Info) const;

     
    bool is_var_const() const;

     
    bool is_var_null() const;

     
    bool is_var_pointer() const;

     
    bool is_var_reference() const;

     
    bool is_var_undef() const;

     
     
     
    string type_name() const;
  };

   
  class Map_Pair {
  public:
     
    const string first();

     
    Object second();
  };

   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  class Map {
  public:
     
    Range range();

     
    Const_Range range() const;

     
    int size() const;

     
    Object operator[](string);

     
    void clear();

     
    int count(string) const;

     
    bool empty() const;
  };

   
  class Container {
  public:
    void push_back(Object);
    Range range();
    Const_Range range() const;
  };

   
   
   
   
   
   
  string to_string(Object o);

   
   
   
   
   
   
   
  void puts(Object o);

   
   
   
   
   
   
   
   
  void print(Object o);

   
   
   
   
   
   
   
   
   
   
   
   
   
  class string {
  public:
     
     
     
     
     
    int find(string s) const;

     
     
     
     
     
    int rfind(string s) const;

     
     
     
     
     
     
    int find_first_of(string list) const;

     
     
     
     
     
     
    int find_last_of(string list) const;

     
     
     
     
     
     
    int find_first_not_of(string list) const;

     
     
     
     
     
     
    int find_last_not_of(string list) const;

     
     
     
     
     
     
     
     
     
     
    string lstrim() const;

     
     
     
     
     
     
     
     
     
     
    string rtrim() const;

     
     
     
     
     
     
     
     
     
     
     
     
    string trim() const;

     
    const char &operator[](int t_index) const;

     
    char &operator[](int t_index);

     
    const char *c_str() const;

     
    const char *data() const;

     
    void clear();

     
    bool empty() const;

     
     
     
     
    int size() const;

     
    Range range();

     
    Const_Range range() const;
  };

   
   
   
   
   
   
  class Range {
  public:
     
    Object back();

     
     
    bool empty() const;

     
    Object front();

     
     
     
    void pop_back();

     
     
     
    void pop_front();
  };

   
   
   
   
   
   
  class Const_Range {
  public:
     
    const Object back();

     
     
    bool empty() const;

     
    const Object front();

     
     
     
    void pop_back();

     
     
     
    void pop_front();
  };

   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  class Vector {
  public:
     
    Object operator[](int t_index);

     
    const Object operator[](int t_index) const;

     
    Object back();

     
    void clear();

     
    bool empty();

     
    void erase_at(int t_index);

     
    Object front();

     
     
     
    void insert_ref_at(int, Object);

     
     
     
    void insert_at(int, Object);

     
    void pop_back();

     
     
     
    void push_back_ref(Object);

     
     
     
    void push_back(Object);

     
    Range range();

     
    Const_Range range() const;

     
    int size() const;
  };

  class Type_Info {
  public:
     
     
    bool bare_equal(Type_Info t_ti) const;

     
    string cpp_bare_name() const;

     
    string cpp_name() const;

     
    bool is_type_const() const;

     
    bool is_type_pointer() const;

     
    bool is_type_reference() const;

     
    bool is_type_undef() const;

     
    bool is_type_void() const;

     
    string name() const;
  };

   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  class Function {
  public:
     
    string get_annotation() const;

     
     
     
     
     
     
     
     
     
    int get_arity() const;

     
     
     
     
     
     
     
     
     
     
     
     
    Vector get_contained_functions() const;

     
     
     
     
     
     
     
     
     
    Function get_guard() const;

     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
    Vector get_param_types() const;

     
    bool has_guard() const;

     
     
     
     
     
     
     
    Object call(Vector t_params) const;
  }

   
   
   
   
   
   
   
   
   
   
   
  Object
  max(Object a, Object b);

   
   
   
   
   
   
   
   
   
   
   
   
  Object min(Object a, Object b);

   
   
   
   
   
   
   
   
   
  bool even(Object x);

   
   
   
   
   
   
   
   
   
  bool even(Object x);

   
   
   
   
   
   
   
   
   
  void for_each(Range c, Function f);

   
   
   
   
   
   
   
  Object map(Range c, Function f);

   
   
   
   
   
   
   
   
  Object foldl(Range c, Function f, Object initial);

   
   
   
   
   
   
   
   
   
   
   
   
  Numeric sum(Range c);

   
   
   
   
   
   
   
   
   
   
   
   
  Numeric product(Range c);

   
   
   
   
   
   
   
   
   
  Object take(Range c, int num);

   
   
   
   
   
   
   
   
   
  Object take_while(Range c, Function f);

   
   
   
   
   
   
   
   
   
  Object drop(Range c, int num);

   
   
   
   
   
   
   
  Object drop_while(Range c, Function f);

   
   
   
   
   
   
   
  Object reduce(Range c, Function f);

   
   
   
   
   
   
   
  Object filter(Container c, Function f);

   
   
   
   
   
   
   
  string join(Range c, string delim);

   
   
   
   
   
   
   
  Container reverse(Container c);

   
   
   
   
   
   
   
   
   
  Vector generate_range(Object x, Object y);

   
   
   
   
   
   
   
  Object concat(Range x, Range y);

   
   
   
   
   
   
   
  Vector collate(Object x, Object y);

   
   
   
   
   
   
   
  Vector zip_with(Function f, Range x, Range y);

   
   
   
   
   
   
   
  Vector zip(Range x, Range y);

   
   
   
   
   
   
   
  bool call_exists(Function f, ...);

   
  Range retro(Range);

   
  Const_Range retro(Const_Range);

   
   
   
   
   
   
   
   
   
  void throw(Object);
}  
