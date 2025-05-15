// *****************************************************************************
// ***   A Little C interpreter   **********************************************
// *****************************************************************************

#define NUM_FUNC    100
#define NUM_VARS    200

class LittleC
{
  public:

    // *************************************************************************
    // ***   Set buffer with program   *****************************************
    // *************************************************************************
    void SetPgmBuffer(char* p_text, int size);

    // *************************************************************************
    // ***   Public: SetOutputBuf   ********************************************
    // *************************************************************************
    bool SetOutputBuf(char* p_buf, int size);

    // *************************************************************************
    // ***   Public: Prescan   *************************************************
    // *************************************************************************
    bool Prescan();

    // *************************************************************************
    // ***   Public: Execute   *************************************************
    // *************************************************************************
    bool Execute();

    // *************************************************************************
    // ***   Public: GetGlobalVariablesCnt   ***********************************
    // *************************************************************************
    int GetGlobalVariablesCnt() {return gvar_index;}

    // *************************************************************************
    // ***   Public: GetGlobalVariableName   ***********************************
    // *************************************************************************
    bool GetGlobalVariableName(int variable_idx, char* ptr, int n);

    // *************************************************************************
    // ***   Public: GetGlobalVariableValue   **********************************
    // *************************************************************************
    bool GetGlobalVariableValue(int variable_idx, int& val);

    // *************************************************************************
    // ***   Public: SetGlobalVariableValue   **********************************
    // *************************************************************************
    bool SetGlobalVariableValue(int variable_idx, int val);

    // *************************************************************************
    // ***   Public: ResetGlobalVariableValue   ********************************
    // *************************************************************************
    bool ResetGlobalVariableValue(int variable_idx);

    // *************************************************************************
    // ***   Public: GetGlobalVariableCommentPtr   *****************************
    // *************************************************************************
    bool GetGlobalVariableCommentPtr(int variable_idx, const char*& ptr);

  private:
    enum tok_types {UNDEFTT, DELIMITER, IDENTIFIER, NUMBER, KEYWORD, TEMP, STRING, BLOCK};

    // Add additional C keyword tokens here
    enum tokens {UNDEFTOK, ARG, VOID, CHAR, INT, IF, ELSE, FOR, DO, WHILE, SWITCH, CASE, DEFAULT, RETURN, CONTINUE, BREAK, END};

    // Add additional double operators here (such as ->)
    enum double_ops {LT = 1, LE, GT, GE, EQ, NE, LS, RS, INC, DEC, ADD, SUB, MUL, DIV, MOD, AND, OR};

    // These are the constants used to call sntx_err() when
    // a syntax error occurs. Add more if you like.
    // NOTE: SYNTAX is a generic error message used when
    // nothing else seems appropriate.
    enum error_msg
    {
      SYNTAX, UNBAL_PARENS, NO_EXP, NOT_VAR, NOT_STRING, PARAM_ERR, SEMI_EXPECTED, UNBAL_BRACES, FUNC_UNDEF, TYPE_EXPECTED,
      NEST_FUNC, RET_NOCALL, PAREN_EXPECTED, WHILE_EXPECTED, QUOTE_EXPECTED, TOO_MANY_LVARS, DIV_BY_ZERO,
      DUP_VAR, DUP_FUNC, TOO_LONG_TOKEN, BRACE_EXPECTED, COLON_EXPECTED, UNDEFINED_TOKEN, END_ERR
    };

    const char* prog;  // current location in source code
    const char* p_buf; // points to start of program buffer

    // Output buffer and size
    char* p_output = nullptr;
    int   output_size = 0;
    int   cur_pos = 0;

    // Token
    char token[80];
    char token_type;
    char tok;
    const char* token_ptr;

    int functos = 0;    // index to top of function call stack
    int func_index = 0; // index into function table
    int gvar_index = 0; // index into global variable table
    int lvartos = 0;    // index into local variable stack

    // Data type structure
    struct data_type
    {
      int type;  // type of variable
      int value; // value of variable
    };

    // Function return value
    data_type ret_data = {0};

    // An array of these structures will hold the info
    // associated with global and local variables
    struct var_type
    {
      const char* name; // pointer to variable name in the program, should point to the first character
      data_type data;   // variable type and data
    };
    // Variables stack
    var_type var_stack[NUM_VARS];

    // Functions table structure
    struct func_type
    {
      const char* func_name; // pointer to function name in the program, should point to the first character
      const char* loc;       // location of entry point in file
      int ret_type;          // return type of function
    };
    // Functions table
    func_type func_table[NUM_FUNC];

    // This is the value used to to find start of all local variables pushed in the function
    int call_stack[NUM_FUNC];

    // Keyword lookup table structure
    struct commands
    {
      const char* command;
      char tok;
    };

    // Keyword lookup table
    const commands table[15] =
    {
      // Commands must be entered lower case in this table.
      {"void", VOID},
      {"char", CHAR},
      {"int", INT},
      {"if", IF},
      {"else", ELSE},
      {"for", FOR},
      {"do", DO},
      {"while", WHILE},
      {"switch", SWITCH},
      {"case", CASE},
      {"default", DEFAULT},
      {"return", RETURN},
      {"continue", CONTINUE},
      {"break", BREAK},
      {"", END}  // mark end of table
    };

    // Error messages lookup table
    struct err_msg
    {
      error_msg err;
      const char* msg;
    };

    // Error messages
    const err_msg errors[26] =
    {
      {SYNTAX,          "Syntax error"},
      {NO_EXP,          "No expression present"},
      {PAREN_EXPECTED,  "Parentheses expected"},
      {QUOTE_EXPECTED,  "Closing quote expected"},
      {TOO_LONG_TOKEN,  "Token is too long"},
      {UNBAL_BRACES,    "Unbalanced braces"},
      {DUP_VAR,         "Duplicate variable name"},
      {DUP_FUNC,        "Duplicate function name"},
      {TYPE_EXPECTED,   "Type specifier expected"},
      {SEMI_EXPECTED,   "Semicolon expected"},
      {NEST_FUNC,       "Too many nested function calls"},
      {RET_NOCALL,      "Return without call"},
      {PARAM_ERR,       "Parameter error"},
      {NOT_VAR,         "Not a variable"},
      {NOT_STRING,      "Not a string"},
      {BRACE_EXPECTED,  "{ expected (control statements must use blocks)"},
      {COLON_EXPECTED,  "Colon expected"},
      {WHILE_EXPECTED,  "While expected"},
      {UNBAL_PARENS,    "Unbalanced parentheses"},
      {FUNC_UNDEF,      "Function undefined"},
      {TOO_MANY_LVARS,  "Too many local variables"},
      {DIV_BY_ZERO,     "Division by zero"},
      {UNDEFINED_TOKEN, "Undefined token"},
      {END_ERR,         "Error Not Found"}
    };

    bool interp_block(void);
    int find_func(const char* name);
    bool decl_global(void);
    bool decl_local(void);
    bool call(data_type& data);
    bool get_args(int& count);
    bool get_params(int count);
    bool func_ret(void);
    bool local_push(var_type i);
    bool func_pop(int& param);
    bool func_push(int i);
    bool assign_var(char* var_name, data_type data);
    bool find_var(char* s, data_type& data);
    bool is_var(char* s);
    bool exec_if(void);
    bool exec_while(void);
    bool exec_do(void);
    bool exec_for(void);
    bool exec_switch(void);
    bool find_eob(void);
    bool eval_exp(data_type& data, bool evaluate_comma = false);
    bool eval_exp00(data_type& data, bool evaluate_comma = true);
    bool eval_exp0(data_type& data);
    bool eval_exp1(data_type& data);
    bool eval_exp2(data_type& data);
    bool eval_exp3(data_type& data);
    bool eval_exp4(data_type& data);
    bool eval_exp5(data_type& data);
    bool atom(data_type& data);
    bool sntx_err(int error);
    bool get_token(void);
    bool get_string_token(int idx);
    void putback(void);
    int look_up(char* s);
    int internal_func(char* s);
    int isdelim(char c);
    int iswhite(char c);
    bool strcomp(const char* str1, const char* str2);

    // "Standard library" functions are declared here so
    // they can be put into the internal function table that
    // follows.
    bool no_arg_func(); // Helper function to prevent copy-paste for functions without arguments

    bool call_putch(data_type&);
    bool call_puts(data_type&);
    bool call_printfp(data_type&);
    bool call_print(data_type&);
    bool call_println(data_type&);
    bool call_abs(data_type&);

    // SmartPendant specific functions
    bool call_getaxisposx(data_type&);
    bool call_getaxisposy(data_type&);
    bool call_getaxisposz(data_type&);
    bool call_islathediametermode(data_type&);

    // Internal functions structure definition
    struct intern_func_type
    {
      const char* f_name;  // function name
      bool (LittleC::*p)(data_type&); // pointer to the function
    };

    // Internal functions table
    static const intern_func_type intern_func[];
};
