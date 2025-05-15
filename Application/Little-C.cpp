// *****************************************************************************
// ***   A Little C interpreter   **********************************************
// *****************************************************************************

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "GrblComm.h" // For internal functions

#include "Little-C.h"

const LittleC::intern_func_type LittleC::intern_func[] =
{
  "putch", &LittleC::call_putch,
  "puts", &LittleC::call_puts,
  "print", &LittleC::call_print,
  "println", &LittleC::call_println,
  "printfp", &LittleC::call_printfp,
  "abs", &LittleC::call_abs,
  "GetAxisPosX", &LittleC::call_getaxisposx,
  "GetAxisPosY", &LittleC::call_getaxisposy,
  "GetAxisPosZ", &LittleC::call_getaxisposz,
  "IsLatheDiameterMode", &LittleC::call_islathediametermode,
  "", 0 // null terminate the list
};

// *****************************************************************************
// ***   Set buffer with program   *********************************************
// *****************************************************************************
void LittleC::SetPgmBuffer(char* p_text, int size)
{
  // Save program pointer
  p_buf = p_text;
  // Clear functions index
  func_index = 0;
  // Clear global variable index
  gvar_index = 0;
}

// *****************************************************************************
// ***   Set output buffer, size and clear current position   ******************
// *****************************************************************************
bool LittleC::SetOutputBuf(char* p_obuf, int size)
{
  p_output = p_obuf;
  output_size = size;
  cur_pos = 0;
  // Return true if buffer is exist
  return ((p_buf != nullptr) && (size != 0));
}

// *****************************************************************************
// ***   Find the location of all functions in    ******************************
// ***   the program and store global variables.   *****************************
// *****************************************************************************
bool LittleC::Prescan()
{
  bool result = false;

  // If we have pointer to buffer with program
  if(p_buf != nullptr)
  {
    // Set result
    result = true;

    // Set program pointer to start of program buffer
    prog = p_buf;

    int brace = 0;  // When 0, this var tells us that current source position is outside of any function.

    // Initialize functions index
    func_index = 0;
    // Initialize global variable index
    gvar_index = 0;
    // Undefined token before prescan
    tok = UNDEFTOK;

    while(result && (tok != END))
    {
      // Bypass code inside functions
      while((brace) && (tok != END))
      {
        result = get_token();
        if(*token == '{') brace++;
        if(*token == '}') brace--;
      }

      // If end reached or bad result - break the cycle
      if((tok == END) || !result) break;

      // Save current position
      const char* tp = prog;
      result = get_token();

      // Is global var
      if((tok == VOID) || (tok == CHAR) || (tok == INT))
      {
        int datatype = tok; // Save data type
        get_token();
        if(token_type == IDENTIFIER)
        {
          const char* fn = token_ptr;
          get_token();
          // Check if it function declaration
          if(*token == '(')
          {
            // See if function already defined.
            for(int i = 0; i < func_index; i++)
              if(!strcomp(func_table[i].func_name, fn))
                result = sntx_err(DUP_FUNC);

            if(result)
            {
              func_table[func_index].loc = prog;
              func_table[func_index].ret_type = datatype;
              func_table[func_index].func_name = fn;
              func_index++;
              while((*token != ')') && (tok != END)) get_token();
              prog++;
              // now prog points to opening curly brace of function
            }
          }
          else // If it not - must be global var
          {
            // Variable couldn't be type VOID
            if(datatype != VOID)
            {
              prog = tp; // return to start of declaration
              decl_global();
            }
            else
            {
              result = sntx_err(TYPE_EXPECTED);
            }
          }
        }
      }
      // Function without declared type
      else if(token_type == IDENTIFIER)
      {
        const char* fn = token_ptr;
        get_token();
        // Check if it function declaration
        if(*token == '(')
        {
          func_table[func_index].loc = prog;
          func_table[func_index].ret_type = VOID; // Void by default if no declared type
          func_table[func_index].func_name = fn;
          func_index++;
          while((*prog != ')') && (*prog != '\0')) prog++;
          prog++;
          // prog points to opening curly brace of function
        }
        else result = sntx_err(TYPE_EXPECTED); // Variables can't be declared without type
      }
      else if(*token == '{') brace++;
      else ; // Do nothing - MISRA rule
    }

    if(result && brace) result = sntx_err(UNBAL_BRACES);
  }

  // Clear number of variables
  if(result == false)
  {
    // Initialize global variable index
    gvar_index = 0;
  }

  return result;
}

// *****************************************************************************
// ***   Public: GetGlobalVariableName   ***************************************
// *****************************************************************************
bool LittleC::GetGlobalVariableName(int variable_idx, char* ptr, int n)
{
  bool result = false;
  // Check if valid variable index passed
  if((variable_idx >= 0) && (variable_idx < gvar_index))
  {
    // Cycle to copy variable name, but not more than passed buffer length
    for(int i = 0; i < n; i++)
    {
      // If delimiter found
      if(isdelim(var_stack[variable_idx].name[i]))
      {
        // Null-terminate buffer, set result abd break the cycle
        ptr[i] = '\0';
        result = true;
        break;
      }
      // Copy character to buffer
      ptr[i] = var_stack[variable_idx].name[i];
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: GetGlobalVariableValue   **************************************
// *****************************************************************************
bool LittleC::GetGlobalVariableValue(int variable_idx, int& val)
{
  bool result = false;
  // Check if valid variable index passed
  if((variable_idx >= 0) && (variable_idx < gvar_index))
  {
    val = var_stack[variable_idx].data.value;
    result = true;
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: SetGlobalVariableValue   **************************************
// *****************************************************************************
bool LittleC::SetGlobalVariableValue(int variable_idx, int val)
{
  bool result = false;
  // Check if valid variable index passed
  if((variable_idx >= 0) && (variable_idx < gvar_index))
  {
    var_stack[variable_idx].data.value = val;
    result = true;
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: ResetGlobalVariableValue   ************************************
// *****************************************************************************
bool LittleC::ResetGlobalVariableValue(int variable_idx)
{
  bool result = false;
  // Check if valid variable index passed
  if((variable_idx >= 0) && (variable_idx < gvar_index))
  {
    result = true;

    // Set prog pointer to global variable
    prog = var_stack[variable_idx].name;

    // Get token to pass variable name pointer
    get_token();
    // Double check that it is variable
    if(token_type != IDENTIFIER) result = sntx_err(SYNTAX);

    if(result)
    {
      // Data. Zero by default.
      data_type data = {0};
      // Another get token to find '=', ',' or ';'
      get_token();
      if(*token == '=') // is an assignment at declaration
      {
        get_token();
        result = eval_exp0(data); // get value and assign
      }
      // If variable declared without value - it should be 0 by default
      var_stack[variable_idx].data.value = data.value;
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: GetGlobalVariableCommentPtr   *********************************
// *****************************************************************************
bool LittleC::GetGlobalVariableCommentPtr(int variable_idx, const char*& ptr)
{
  bool result = false;

  // Check if valid variable index passed
  if((variable_idx >= 0) && (variable_idx < gvar_index))
  {
    const char* var_ptr = var_stack[variable_idx].name;

    // Find comment start
    while((*var_ptr != '\n') && (*var_ptr != '\r') && (*var_ptr != '\0') && (*var_ptr != '/')) var_ptr++;

    if((var_ptr[0] == '/') && (var_ptr[1] == '/'))
    {
      // Skip '//'
      var_ptr += 2;
      // Skip any white spaces
      while(iswhite(*var_ptr)) var_ptr++;

      // If it isn't new line or null-terminatoir character
      if((*var_ptr != '\r') && (*var_ptr != '\n') && (*var_ptr != 0))
      {
        // Set pointer
        ptr = var_ptr;
        // Set result flag
        result = true;
      }
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Execute a program   ***************************************************
// *****************************************************************************
bool LittleC::Execute()
{
  bool result = false;
  // For data returned from main()
  data_type data;

  // Set program pointer to start of program buffer
  prog = p_buf;
  // Set token to undefined before start
  tok = UNDEFTOK;

  // Initialize the CALL stack index
  functos = 0;
  // Initialize local variable stack index
  lvartos = gvar_index;
  // Clear ret value
  ret_data = {0};

  // Setup call to main()
  int idx = find_func("main");  // find program starting point
  if(idx != -1)
  {
    prog = func_table[idx].loc;
    prog--; // back up to opening '('
    strncpy(token, "main", sizeof(token));
    result = call(data);  // call main() to start interpreting
    // Check result If we filled whole buffer
    if(cur_pos >= output_size - 1)
    {
      // Set an error message
      if(p_output != nullptr) snprintf(p_output, output_size, "Program result doesn't fit into buffer");
      // And clear result flag
      result = false;
    }
    else
    {
      // Null-terminate output buffer
      p_output[cur_pos] = '\0';
    }
  }
  else
  {
    if(p_output != nullptr) snprintf(p_output, output_size, "main() not found");
  }

  // Deinitialize local variable stack index to allow ResetGlobalVariableValue() call
  lvartos = 0;

  return result;
}

// *****************************************************************************
// ***   Interpret a single statement or block of code. When       *************
// ***   interp_block() returns from its initial call, the final   *************
// ***   brace (or a return) in main() has been encountered.       *************
// *****************************************************************************
bool LittleC::interp_block(void)
{
  bool result = true;

  // Block flag
  bool block = false;

  // Save local var stack index
  int lvartemp = lvartos;

  do
  {
    result = get_token();

    // If bad result - break the cycle
    if(result == false) break;

    // See what kind of token is up
    if((token_type == IDENTIFIER) || (*token == INC) || (*token == DEC) || (*token == '(')) // Not a keyword, so process expression.
    {
      data_type data = {0};
      result = eval_exp00(data); // process the expression
      if((result) && (*token != ';')) result = sntx_err(SEMI_EXPECTED);
    }
    else if(token_type == BLOCK) // if block delimiter
    {
      // If end of block - break the cycle
      if(*token == '}')
      {
        break;
      }
      else if(block) // block inside other block
      {
        putback(); // restore token to input stream for further processing by interp_block()
        result = interp_block();
      }
      else
      {
        block = true; // is a block - interpreting block, not statement
      }
    }
    else // is keyword
    {
      switch(tok)
      {
        case CHAR:
        case INT:       // declare local variables
          putback();
          result = decl_local();
          break;
        case RETURN:    // return from function call
          result = func_ret();
          tok = RETURN; // Restore tok after return function evaluation
          break;
        case IF:        // process an if statement
          result = exec_if();
          break;
        case ELSE:      // process an else statement
          result = find_eob(); // find end of else block and continue execution
          break;
        case WHILE:     // process a while loop
          result = exec_while();
          break;
        case DO:        // process a do-while loop
          result = exec_do();
          break;
        case FOR:       // process a for loop
          result = exec_for();
          break;
        case CONTINUE:  // continue loop execution
          break;
        case BREAK:     // break loop execution
          break;
        case SWITCH:  // handle a switch statement 
          result = exec_switch();
          break; 
        case END:
          result = false;
          break;
      }
    }
  } while(result && block && (tok != END) && (tok != RETURN) && (tok != CONTINUE) && (tok != BREAK));

  // Reset the local var stack
  lvartos = lvartemp;

  return result;
}

// *****************************************************************************
// ***   Return the entry point of the specified function.   *******************
// ***   Return NULL if not found.                           *******************
// *****************************************************************************
int LittleC::find_func(const char *name)
{
  int idx = -1;

  for(register int i = 0; i < func_index; i++)
  {
    if(!strcomp(name, func_table[i].func_name))
    {
      idx = i;
      break;
    }
  }

  return idx;
 }

// *****************************************************************************
// ***   Declare a global variable   *******************************************
// *****************************************************************************
bool LittleC::decl_global(void)
{
  bool result = true;

  get_token();  // get type

  int vartype = tok; // save var type

  // Process comma-separated list
  do
  {
    var_stack[gvar_index].data.type = vartype;
    var_stack[gvar_index].data.value = 0;  // init to 0
    get_token(); // Get token to get variable name pointer
    var_stack[gvar_index].name = token_ptr; // Save pointer to variable name

    if(token_type != IDENTIFIER) result = sntx_err(SYNTAX);

    // See if variable is a duplicate
    for(int i = 0; i < gvar_index; i++)
      if(!strcomp(var_stack[i].name, token_ptr))
        result = sntx_err(DUP_VAR);

    if(result)
    {
      get_token(); // Another get token to find '=', ',' or ';'
      if(*token == '=') // is an assignment at declaration
      {
        data_type data = {0};
        get_token();
        result = eval_exp0(data); // get value and assign
        var_stack[gvar_index].data.value = data.value;
      }
      gvar_index++;
    }
  } while(result && (*token == ','));

  if(result && (*token != ';')) result = sntx_err(SEMI_EXPECTED);

  return result;
}

// *****************************************************************************
// ***   Declare a local variable   ********************************************
// *****************************************************************************
bool LittleC::decl_local(void)
{
  bool result = true;

  // Get variable type
  get_token();

  // Variable struct to add into stack
  var_type var = {nullptr, {tok, 0}};

  // Process comma-separated list
  do
  {
    var.data.value = 0; // init to 0
    get_token(); // Get token to get variable name pointer
    if(token_type != IDENTIFIER) result = sntx_err(SYNTAX);
    if(result)
    {
      var.name = token_ptr; // Save pointer to variable name
      get_token(); // Another get token to find '=', ',' or ';'
      if(*token == '=') // is an assignment at declaration
      {
        data_type data = { 0 };
        get_token();
        result = eval_exp0(data); // get value and assign
        var.data.value = data.value;
      }
    }
    if(result) result = local_push(var);
  } while(result && (*token==','));

  if(result && (*token != ';')) result = sntx_err(SEMI_EXPECTED);

  return result;
}

// *****************************************************************************
// ***   Call a function   *****************************************************
// *****************************************************************************
bool LittleC::call(data_type& data)
{
  bool result = true;

  int idx = find_func(token); // find entry point of function
  if(idx == -1)
  {
    result = sntx_err(FUNC_UNDEF); // function not defined
  }
  else
  {
    int arg_count = 0;
    int lvartemp = lvartos;  // save local var stack index
    result = get_args(arg_count);  // get function arguments
    const char* temp = prog; // save return location
    if(result)
    {
      result = func_push(lvartemp);  // save local var stack index
      prog = func_table[idx].loc;  // reset prog to start of function
    }
    if(result) result = get_params(arg_count); // load the function's parameters with the values of the arguments
    if(result) result = interp_block(); // interpret the function
    if(result)
    {
      data = ret_data; // Copy return value
      data.type = func_table[idx].ret_type; // Set type from function prototype
      prog = temp; // reset the program pointer
      result = func_pop(lvartos); // reset the local var stack
    }
  }

  return result;
}

// *****************************************************************************
// ***   Push the arguments to a function onto the local variable stack   ******
// *****************************************************************************
bool LittleC::get_args(int& count)
{
  bool result = true;
  struct var_type var = {"", ARG, 0};

  // Clear arguments count
  count = 0;

  // Get first token and check if it '('
  result = get_token();
  if(result && (*token != '(')) result = sntx_err(PAREN_EXPECTED);

  if(result)
  {
    // Get another token to check if it is ')' - if it is, function has no arguments
    result = get_token();
    if(result && (*token != ')'))
    {
      // If function has arguments and token isn't ')' - put it back
      putback();
      // Process a comma-separated list of values
      do
      {
        data_type data = {0};
        result = eval_exp(data);
        var.data.value = data.value;
        if(result) result = local_push(var);
        if(result) result = get_token();
        count++;
      } while(result && (*token == ','));
    }
  }

  if(result && (*token != ')')) result = sntx_err(PAREN_EXPECTED);

  return result;
}

// *****************************************************************************
// ***   Get function parameters   *********************************************
// *****************************************************************************
bool LittleC::get_params(int count)
{
  bool result = true;

  // Find first argument on the stack
  int si = lvartos - count;

  // Clear token to enter the cycle
  *token = '\0';

  // No arguments case
  if(count == 0) get_token();

  // Process comma-separated list of parameters. Stop when either
  // all arguments processed or ')' reached.
  for(int i = 0; (i < count) && (*token != ')'); i++, si++)
  {
    get_token();
    // Check token type - should be type token
    if((tok != INT) && (tok != CHAR))
    {
      result = sntx_err(TYPE_EXPECTED);
      break;
    }
    var_stack[si].data.type = tok; // Set argument type
    get_token(); // Get token to get variable name pointer
    var_stack[si].name = token_ptr; // Save pointer to variable name
    get_token(); // Another get token followed after variable name
  }

  // Check that number of passed arguments match function arguments
  if(result)
  {
    if(si != lvartos)      result = sntx_err(PARAM_ERR);
    else if(*token != ')') result = sntx_err(PAREN_EXPECTED);
    else ; // Do nothing - MISRA rule
  }

  return result;
}

// *****************************************************************************
// ***   Return from a function   **********************************************
// *****************************************************************************
bool LittleC::func_ret(void)
{
  // Get return value, if any(including comma operator)
  return eval_exp(ret_data, true);
}

// *****************************************************************************
// ***   Push a local variable   ***********************************************
// *****************************************************************************
bool LittleC::local_push(LittleC::var_type i)
{
  bool result = true;

  if(lvartos >= NUM_VARS)
  {
    result = sntx_err(TOO_MANY_LVARS);
  }
  else
  {
    var_stack[lvartos] = i;
    lvartos++;
  }

  return result;
}

// *****************************************************************************
// ***   Pop index into local variable stack   *********************************
// *****************************************************************************
bool LittleC::func_pop(int &param)
{
  bool result = true;

  functos--;
  if(functos < 0)              result = sntx_err(RET_NOCALL);
  else if(functos >= NUM_FUNC) result = sntx_err(NEST_FUNC);
  else param = call_stack[functos];

  return result;
}

// *****************************************************************************
// ***   Push index of local variable stack   **********************************
// *****************************************************************************
bool LittleC::func_push(int i)
{
  bool result = true;
  if(functos >= NUM_FUNC)
  {
    result = sntx_err(NEST_FUNC);
  }
  else
  {
    call_stack[functos] = i;
    functos++;
  }
  return result;
}

// *****************************************************************************
// ***   Assign a value to a variable   ****************************************
// *****************************************************************************
bool LittleC::assign_var(char *var_name, data_type data)
{
  bool result = false;

  // First, see if it's a local variable
  for(register int i = lvartos - 1; i >= 0; i--)
  {
    // Check variable name
    if(!strcomp(var_stack[i].name, var_name))
    {
      if(var_stack[i].data.type == CHAR) var_stack[i].data.value = (char)data.value;
      else if(var_stack[i].data.type == INT) var_stack[i].data.value = (int)data.value;
      else var_stack[i].data.value = data.value;
      result = true;
      break;
    }
    // If not local, try global variables array portion
    if(i == (call_stack[functos - 1])) i = gvar_index;
  }

  if(result == false) sntx_err(NOT_VAR); // Variable not found

  return result;
}

// *****************************************************************************
// ***   Find the value of a variable   ****************************************
// *****************************************************************************
bool LittleC::find_var(char* var_name, data_type& data)
{
  bool result = false;

  // First, see if it's a local variable
  for(register int i = lvartos - 1; i >= 0; i--)
  {
    if(!strcomp(var_stack[i].name, var_name))
    {
      data = var_stack[i].data;
      result = true;
      break;
    }
    // If not local, try global variables array portion
    if(i == (call_stack[functos - 1])) i = gvar_index;
  }

  if(result == false) sntx_err(NOT_VAR); // Variable not found

  return result;
}

// *****************************************************************************
// ***   Determine if an identifier is a variable.     *************************
// ***   Return 1 if variable is found; 0 otherwise.   *************************
// *****************************************************************************
bool LittleC::is_var(char *var_name)
{
  bool result = false;

  // First, see if it's a local variable
  for(register int i = lvartos - 1; i >= 0; i--)
  {
    // Check variable name
    if(!strcomp(var_stack[i].name, var_name))
    {
      result = true;
      break;
    }
    // If not local, try global variables array portion
    if(i == (call_stack[functos - 1])) i = gvar_index;
  }

  return result;
}

// *****************************************************************************
// ***   Execute an if statement   *********************************************
// *****************************************************************************
bool LittleC::exec_if(void)
{
  bool result = true;

  // Data type to evaluate condition
  data_type cond;
  // Evaluate condition(including comma operator)
  result = eval_exp(cond, true);

  if(result)
  {
    // is true so process target of IF
    if(cond.value)
    {
      result = interp_block();
    }
    else // otherwise skip around IF block and process the ELSE, if present
    {
      result = find_eob(); // find start of next line
      if(result) result = get_token();

      if(result)
      {
        if(tok != ELSE)
        {
          putback();  // restore token if no ELSE is present
        }
        else
        {
          result = interp_block();
        }
      }
    }
  }

  return result;
}

// *****************************************************************************
// ***   Execute a while loop   ************************************************
// *****************************************************************************
bool LittleC::exec_while(void)
{
  bool result = true;

  // Data type to evaluate condition
  data_type cond;

  putback();   // put back while
  const char* temp = prog; // save location of top of while loop
  result = get_token(); // past while
  if(result && (tok != WHILE)) result = sntx_err(WHILE_EXPECTED);
  if(result) result = eval_exp(cond, true); // check the conditional expression(including comma operator)

  // Only if successful result
  if(result)
  {
    // If condition true - process cycle
    if(cond.value)
    {
      // Save program pointer after eval_exp() in case of break
      const char* temp_ret = prog;
      // Process block
      result = interp_block();  // if true, interpret
      // If break happened
      if((tok == RETURN) || (tok == BREAK))
      {
        // Restore program pointer
        prog = temp_ret;
        // Clear condition to end the loop
        cond.value = 0;
      }
      else
      {
        prog = temp;  // loop back to top
      }
    }
    else // otherwise, skip around loop
    {
      result = find_eob();
    }
  }
 
  return result;
}

// *****************************************************************************
// ***   Execute a do loop   ***************************************************
// *****************************************************************************
bool LittleC::exec_do(void)
{
  bool result = true;
  // Continue flag for break the cycle in case of break or return
  bool cont = true;

  putback();
  const char* temp = prog;  // save location of top of do loop

  get_token(); // get start of loop
  result = interp_block(); // interpret loop
  if((tok == CONTINUE) || (tok == BREAK) || (tok == RETURN))
  {
    if ((tok == BREAK) || (tok == RETURN))
    {
      cont = false;
    }
    // Search end of while cycle
    while(tok != WHILE)
    {
      get_token();
      if(tok == END)
      {
        result = sntx_err(SYNTAX);
        break;
      }
    }
    if(result) putback();
  }
  if(result)
  {
    data_type cond; // data type to evaluate condition
    if(result) result = get_token();
    if(result && (tok != WHILE)) result = sntx_err(WHILE_EXPECTED);
    if(result) result = eval_exp(cond, true); // check the loop condition including comma operator
    if(result && cond.value && cont) prog = temp; // if true loop; otherwise, continue on
  }

  return result;
}

// *****************************************************************************
// ***   Execute a for loop   **************************************************
// *****************************************************************************
bool LittleC::exec_for(void)
{
  bool result = true;
  // Data type to evaluate condition
  data_type cond = {0};

  // Save local var stack index
  int lvartemp = lvartos;

  // To pass opening '('
  result = get_token();
  if(result && (*token != '(')) result = sntx_err(PAREN_EXPECTED);
  // To figure out next token type
  result = get_token();

  // Check token type - if it char or int
  if((tok == CHAR) || (tok == INT))
  {
    // Putback type token
    putback();
    // Declare local variable
    result = decl_local();
  }
  else
  {
    // Otherwise initialization expression(s)
    result = eval_exp00(cond);
  }

  if(result)
  {
    if(*token != ';')
    {
      result = sntx_err(SEMI_EXPECTED);
    }
    else
    {
      const char* temp1 = prog;
      for(;result;)
      {
        get_token();
        result = eval_exp00(cond);  // check the condition
        if(result)
        {
          if(*token != ';')
          {
            result = sntx_err(SEMI_EXPECTED);
          }
          else
          {
            const char* temp2 = prog;

            // find the start of the for block
            int brace = 1;
            while(brace)
            {
              get_token();
              if(*token=='(') brace++;
              if(*token==')') brace--;
              if(tok == END)
              {
                result = sntx_err(PAREN_EXPECTED);
                break;
              }
            }

            if(result)
            {
              if(cond.value) // if true, interpret
              {
                // Save pointer in case of break
                const char* temp3 = prog;
                // Process block
                result = interp_block();
                // Check result
                if(result)
                {
                  // If break happened
                  if((tok == BREAK) || (tok == RETURN))
                  {
                    // Restore program pointer
                    prog = temp3;
                    // Clear condition to end the loop
                    cond.value = 0;
                  }
                }
              }
              // Otherwise, skip around loop - should bot be else to handle break & return !
              if(!cond.value)
              {
                result = find_eob();
                break;
              }
            }
            if(result)
            {
              prog = temp2;
              get_token();
              result = eval_exp00(cond); // do the increment, or multiple if comma separated
              prog = temp1;  // loop back to top
            }
          }
        }
      }
    }
  }

  // Reset the local var stack
  lvartos = lvartemp;

  return result;
}

// *****************************************************************************
// ***   Execute a switch statement   ******************************************
// *****************************************************************************
bool LittleC::exec_switch(void)
{ 
  bool result = true;
  data_type sval, cval;

  // Get switch expression
  result = eval_exp(sval);

  // Since eval_exp() will putback last token - take it again
  get_token();
  // Check for start of block
  if(result && (*token != '{')) result = sntx_err(BRACE_EXPECTED);

  // Save local var stack index
  int lvartemp = lvartos;

  // Set brace counter
  int brace = 1;

  // Now, check case statements
  for(;;)
  {
    // Find a case statement
    do
    {
      get_token();
      if(*token == '{') brace++;
      else if(*token == '}') brace--;
      else ; // Do nothing - MISRA rule

      // Check for program end
      if(tok == END)
      {
        result = sntx_err(SYNTAX);
        break;
      }
    } while(((tok != CASE) && (tok != DEFAULT) && brace) || (brace > 1)); // Ignore case from nested statemets

    // If no matching case found, then skip
    if(!brace) break; 

    // Get value of the case statement
    if(result)
    {
      if(tok == DEFAULT) cval = sval; // Make it match to enter default statement
      else result = eval_exp(cval);
    }

    // Read and discard the ':'
    get_token(); 
    if(result && (*token != ':')) result = sntx_err(COLON_EXPECTED);

    // If values match, then interpret. 
    if(result && (cval.value == sval.value))
    {
      do
      {
        result = interp_block();

        get_token();
        if(*token == '}') brace--; // brace always should be 0 at this point
        putback();
      } while((tok != BREAK) && (tok != END) && brace);

      // Find end of switch statement
      while(brace)
      {
        get_token();
        if(*token == '{') brace++;
        else if(*token == '}') brace--;
        else ; // Do nothing - MISRA rule
      }
      break;
    }

    if(result == false) break;
  }

  // Reset the local var stack
  lvartos = lvartemp;

  return result;
}

// *****************************************************************************
// ***   Find the end of a block   *********************************************
// *****************************************************************************
bool LittleC::find_eob(void)
{
  bool result = true;
  bool ret = false;

  // If current tok is RETURN, we have to preserve it
  if(tok == RETURN) ret = true;

  int brace = 0;
  int parenthesis = 0;
  bool semicolon = false;

  while(true)
  {
    get_token();
    if(tok == END)
    {
      result = sntx_err(SYNTAX);
      break;
    }
    else if(*token == '(') parenthesis++;
    else if(*token == ')') parenthesis--;
    else if(*token == '{') brace++;
    else if(*token == '}') brace--;
    else if((*token == ';') && (parenthesis == 0)) semicolon = true; // Semicolon shoudn't be inside() to be count
    else ; // Do nothing - MISRA rule

    // If semicolon found and we not in braces and not in parenthesis - we found end of the block
    if(semicolon && (brace == 0) && (parenthesis == 0)) break;

    // Brace count can't be less than 0
    if(brace < 0)
    {
      result = sntx_err(UNBAL_BRACES);
      break;
    }
    // Parenthesis count can't be less than 0
    if(parenthesis < 0)
    {
      result = sntx_err(UNBAL_PARENS);
      break;
    }
  }

  // Restore return tok if needed
  if(ret && result) tok = RETURN;

  return result;
}

// *****************************************************************************
// *****************************************************************************
// ***   PARSER.cpp   **********************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Entry point into parser with processing comma operators  **************
// *****************************************************************************
bool LittleC::eval_exp(data_type& data, bool evaluate_comma)
{
  bool result = get_token();
  if(result) result = eval_exp00(data, evaluate_comma); // Do not process comma operator
  if(result) putback(); // return last token read to input stream
  return result;
}

// *****************************************************************************
// ***   Entry point into parser   *********************************************
// *****************************************************************************
bool LittleC::eval_exp00(data_type& data, bool evaluate_comma)
{
  bool result = true;

  if(!*token)
  {
    result = sntx_err(NO_EXP);
  }
  else if(*token == ';')
  {
    // Empty expression
    data.type = VOID;
    data.value = 0;
  }
  else
  {
    result = eval_exp0(data);
    // Process comma only for expression in () or inside for()
    while(result && (*token == ',') && evaluate_comma)
    {
      get_token(); // to pass the ','
      result = eval_exp0(data);
    }
  }

  return result;
}

// *****************************************************************************
// ***   Process an assignment expression   ************************************
// *****************************************************************************
bool LittleC::eval_exp0(data_type& data)
{
  bool result = true;
  bool ret = false;

  if(token_type == IDENTIFIER)
  {
    if(is_var(token)) // if a var, see if assignment
    {
      // Holds name of var receiving the assignment
      char temp[sizeof(token)];
      strncpy(temp, token, sizeof(temp));
      temp[sizeof(temp) - 1] = '\0';
      // Get token to figure out if it is an assignment operation
      get_token();
      register char op = *token;
      if((op == '=') || (op == ADD) || (op == SUB) || (op == MUL) || (op == DIV) || (op == MOD))
      {
        result = find_var(temp, data); // get var's value
        if(result)
        {
          data_type val = {0};
          get_token();
          result = eval_exp0(val);  // get value to process
          switch(op)
          {
            case ADD:
              data.value += val.value;
              break;
            case SUB:
              data.value -= val.value;
              break;
            case MUL:
              data.value *= val.value;
              break;
            case DIV:
              if(val.value == 0) result = sntx_err(DIV_BY_ZERO);
              else data.value /= val.value;
              break;
            case MOD:
              if(val.value == 0) result = sntx_err(DIV_BY_ZERO);
              else data.value %= val.value;
              break;
            default:
              data.value = val.value; // assignment
          }
          if(result) result = assign_var(temp, data);  // assign the value
        }
        // Set flag to not to call eval_exp1()
        ret = true;
      }
      else // not an assignment
      {
        // Restore original token
        putback();
        strncpy(token, temp, sizeof(token));
        token_type = IDENTIFIER;
      }
    }
  }

  // Process logical operators
  if(ret == false)
  {
    data_type partial_data = {0};
    result = eval_exp1(data);
    register char op = *token;
    if(result)
    {
      if((op == AND) || (op == OR))
      {
        get_token();
        result = eval_exp0(partial_data);
        switch(op)
        { // Perform the logical operation
          case AND:
            data.value = (data.value && partial_data.value);
            break;
          case OR:
            data.value = (data.value || partial_data.value);
            break;
        }
      }
      if(op == '?')
      {
        // Check that we found '?'
        if(result && (*token != '?')) result = sntx_err(PARAM_ERR);
        // Check result
        if(result)
        {
          // Check value
          if(data.value)
          {
            // Get token to pass '?'
            if(result) result = get_token();
            // Get value. Not including comma operator because ternary
            // operator can be used as function parameter and in this case
            // comma isn't operator but delimiter. comma can be an operator
            // in the first ternary operator part, but to keep everything
            // similar it won't.
            if(result) result = eval_exp0(data);
            // Check that we found ':'
            if(result && (*token != ':')) result = sntx_err(PARAM_ERR);
            // Flag to skip first expression - we should ignore any ':' inside braces or parenthesis
            int parenthesis = 0;
            int brace = 0;
            int ternary = 0;
            // Skip second expression
            while(result && (((*token != ';') && (*token != ',') && (*token != '}')) || parenthesis || brace || ternary) && (*token != '\0'))
            {
              result = get_token();
              if     (*token == '(') parenthesis++;
              else if(*token == ')') parenthesis--;
              else if(*token == '{') brace++;
              else if(*token == '}') brace--;
              else if(*token == '?') ternary++;
              else if(*token == ':') ternary--;
              else; // Do nothing - MISRA rule
              // We can have -1 if ternary operator enclosed in parenthesis or inside another ternary operator
              if((parenthesis < 0) || (ternary < 0)) break;
            }
          }
          else
          {
            // Variables to count
            int parenthesis = 0;
            int brace = 0;
            int ternary = 1;
            // Skip first expression
            while(result && (parenthesis || brace || ternary) && (*token != '\0'))
            {
              result = get_token();
              if     (*token == '(') parenthesis++;
              else if(*token == ')') parenthesis--;
              else if(*token == '{') brace++;
              else if(*token == '}') brace--;
              else if(*token == '?') ternary++;
              else if(*token == ':') ternary--;
              else ; // Do nothing - MISRA rule
              // Should never get below 0
              if(parenthesis < 0) result = sntx_err(UNBAL_PARENS);
              // Should never get below 0
              if(ternary < 0) result = sntx_err(PARAM_ERR);
            }
            // Check that we found ':'
            if(result && (*token != ':')) result = sntx_err(PARAM_ERR);
            // Get token to pass ':'
            if(result) result = get_token();
            // Get another value. Not including comma operator because ternary
            // operator can be used as function parameter and in this case
            // comma isn't operator but delimiter
            if(result) result = eval_exp0(data);
          }
        }
      }
    }
  }

  return result;
}

// *****************************************************************************
// ***   Process relational operators   ****************************************
// *****************************************************************************
bool LittleC::eval_exp1(data_type& data)
{
  bool result = true;
  data_type partial_data = {0};

  static const char relops[7] = {LT, LE, GT, GE, EQ, NE, 0};

  result = eval_exp2(data);
  register char op = *token;
  if(result && strchr(relops, op))
  {
    get_token();
    result = eval_exp2(partial_data);
    switch(op)
    { // Perform the relational operation
      case LT:
        data.value = data.value < partial_data.value;
        break;
      case LE:
        data.value = data.value <= partial_data.value;
        break;
      case GT:
        data.value = data.value > partial_data.value;
        break;
      case GE:
        data.value = data.value >= partial_data.value;
        break;
      case EQ:
        data.value = data.value == partial_data.value;
        break;
      case NE:
        data.value = data.value != partial_data.value;
        break;
    }
  }
  return result;
}

// *****************************************************************************
// ***   Add or subtract two terms   *******************************************
// *****************************************************************************
bool LittleC::eval_exp2(data_type& data)
{
  bool result = true;
  register char  op;
  data_type partial_data;

  static const char okops[] = {'(', INC, DEC, '-', '+', 0};

  result = eval_exp3(data);
  while(result && (((op = *token) == '+') || (op == '-')))
  {
    get_token();

    if(token_type == DELIMITER && !strchr(okops, *token))
    {
      result = sntx_err(SYNTAX);
    }

    if(result) result = eval_exp3(partial_data);

    if (result)
    {
      switch(op)
      {
        // ***   Add   *********************************************************
        case '-':
          data.value -= partial_data.value;
          break;
        // ***   Subtract   ****************************************************
        case '+':
          data.value += partial_data.value;
          break;
      }
    }
  }
  return result;
}

// *****************************************************************************
// ***   Multiply or divide two factors   **************************************
// *****************************************************************************
bool LittleC::eval_exp3(data_type& data)
{
  bool result = true;
  register char op;
  data_type partial_data;

  static const char okops[] = {'(', INC, DEC, '-', '+', 0};

  result = eval_exp4(data);
  while(result && (((op = *token) == '*') || (op == '/') || (op == '%')))
  {
    get_token();

    if(token_type == DELIMITER && !strchr(okops, *token))
    {
      result = sntx_err(SYNTAX);
    }

    if(result) result = eval_exp4(partial_data);

    if(result)
    {
      switch(op)
      {
        // ***   Multiplication   ************************************************
        case '*':
          data.value *= partial_data.value;
          break;
        // ***   Division   ******************************************************
        case '/':
          if(partial_data.value == 0) result = sntx_err(DIV_BY_ZERO);
          else data.value /= partial_data.value;
          break;
        // ***   Modulus   *******************************************************
        case '%':
          if(partial_data.value == 0) result = sntx_err(DIV_BY_ZERO);
          else data.value %= partial_data.value;
          break;
      }
    }
  }
  return result;
}

// *****************************************************************************
// ***   Is a unary +, -, ++, -- or logic ! operation   ************************
// *****************************************************************************
bool LittleC::eval_exp4(data_type& data)
{
  bool result = true;
  register char op = '\0';

  if((*token == '+') || (*token == '-') || (*token == '!') || (*token == INC) || (*token == DEC))
  {
    op = *token;
    get_token();
    if((op == INC) || (op == DEC))
    {
      data_type val = {0};
      result = find_var(token, val);
      if(result)
      {
        if(op == INC) val.value++;
        else          val.value--;
        result = assign_var(token, val);
      }
    }
  }

  if(result) result = eval_exp5(data);

  if(op == '-') data.value = -(data.value);
  if(op == '!') data.value = !(data.value);

  return result;
}

// *****************************************************************************
// ***   Process parenthesized expression   ************************************
// *****************************************************************************
bool LittleC::eval_exp5(data_type& data)
{
  bool result = true;

  if(*token == '(')
  {
    get_token();
    result = eval_exp00(data); // get subexpression
    if(result && (*token != ')')) result = sntx_err(PAREN_EXPECTED);
    get_token();
  }
  else
  {
    result = atom(data);
  }

  return result;
}

// *****************************************************************************
// ***   Find value of number, variable, or function   *************************
// *****************************************************************************
bool LittleC::atom(data_type& data)
{
  bool result = true;

  int i;

  switch(token_type)
  {
    case IDENTIFIER:
      i = internal_func(token);
      if(i != -1) // call "standard library" function
      {
        result = (*this.*intern_func[i].p)(data);
      }
      else if(find_func(token) >= 0) // call user-defined function
      {
        result = call(data);
      }
      else
      {
        result = find_var(token, data); // get var's value

        // Save variable name
        char temp[sizeof(token)];
        strncpy(temp, token, sizeof(temp));
        temp[sizeof(temp) - 1] = '\0';

        get_token();
        if((*token == INC) || (*token == DEC))
        {
          data_type val = data;
          if(*token == INC) val.value++;
          else              val.value--;
          result = assign_var(temp, val);
        }
        else putback();
      }
      get_token();
      break;

    case NUMBER: // is numeric constant
      data.type = INT;
      data.value = atoi(token);
      get_token();
      break;

    case STRING: // is string constant
      data.type = STRING;
      data.value = (int)(token_ptr - p_buf);
      get_token();
      break;

    case DELIMITER: // see if character constant or empty expression
      if((*token == '\'') || (*token == ')'))
      {
        if(*token == '\'') // if character constant
        {
          data.type = CHAR;
          data.value = *prog;
          prog++; // Pass the character
          if(*prog != '\'') result = sntx_err(QUOTE_EXPECTED);
          prog++; // Pass the quote mark
          get_token();
        }
        break;
      }
      // Intentional fall-trough to default to in case of error

    default:
      result = sntx_err(SYNTAX); // syntax error
  }

  return result;
}

// *****************************************************************************
// ***   Display an error message   ********************************************
// *****************************************************************************
bool LittleC::sntx_err(int error)
{
  if((p_output != nullptr) && (output_size != 0))
  {
    int linecount = 0;

    // Find error message
    for(int i = 0; i < (int)(sizeof(errors) / sizeof(errors[0])); i++)
    {
      if(errors[i].err == error)
      {
        error = i;
        break;
      }
    }
    // Check that input parameter is an actual error
    if(error > END_ERR) error = END_ERR;

    // Clear current position to output error instead program
    cur_pos = 0;

    // Display error and line number
    snprintf(&p_output[cur_pos], output_size - cur_pos, "%s", errors[error].msg);
    // Move current position to the end
    while((p_output[cur_pos] != '\0') && (cur_pos < output_size)) cur_pos++;

    // Find line number of error
    const char* p = p_buf;
    while((p != prog) && (*p != '\0'))
    {
      p++;
      // Windows or Mac newline
      if(*p == '\r')
      {
        linecount++;
        if(p == prog) break;
        // See if this is a Windows or Mac newline
        if(*(p+1) == '\n') p++;
      }
      // Unix newline
      else if(*p == '\n')
      {
        linecount++;
      }
      else if(*p == '\0') linecount++;
      else ; // Do nothing - MISRA rule
    }
    snprintf(&p_output[cur_pos], output_size - cur_pos, " in line %d\n", linecount);
    // Move current position to the end
    while((p_output[cur_pos] != '\0') && (cur_pos < output_size)) cur_pos++;

    const char* temp = p;
    // Roll back until previous line
    while((p > p_buf) && (*p != '\n') && (*p != '\r')) p--;
    // Skip new line symbol we found and any white spaces
    if((*p == '\n') || (*p == '\r') || iswhite(*p)) p++;
    // Display offending line
    while(p <= temp) p_output[cur_pos++] = *p++;
    p_output[cur_pos] = '\0';
  }

  return false;
}

// *****************************************************************************
// ***   Get a token   *********************************************************
// *****************************************************************************
bool LittleC::get_token(void)
{
  bool result = true;

  register char *temp;

  token_type = UNDEFTT;
  tok = UNDEFTOK;
  token_ptr = nullptr;

  temp = token;
  *temp = '\0';

  // Skip over white spaces and new lines
  while((iswhite(*prog) || (*prog == '\r') || (*prog == '\n')) && *prog) prog++;

  // Skip over white spaces and new lines
  while(*prog)
  {
    // Skip over white spaces and new lines
    if(iswhite(*prog) || (*prog == '\r') || (*prog == '\n'))
    {
      prog++;
      continue;
    }
    // Skip multiline comments
    if((*prog == '/') && (*(prog + 1) == '*'))
    {
      while(!((*prog == '*') && (*(prog + 1) == '/')) && (*prog != '\0')) prog++;
      // Skip comment end
      if(*prog != '\0')
      {
        prog++; // prog will be increased after break
        prog++; // prog will be increased after break
      }
      continue;
    }
    // Look for single line comments
    if((*prog == '/') && (*(prog + 1) == '/'))
    {
      prog += 2;
      while((*prog != '\n') && (*prog != '\r') && (*prog != '\0')) prog++; // Find end of the line
      continue;
    }
    // If we did not enter in any of conditions above - we should exit this cycle
    break;
  }

  // End of file
  if(*prog == '\0')
  {
    *token = '\0';
    tok = END;
    token_type = DELIMITER;
  }
  // Block delimiters
  else if(strchr("{}", *prog))
  {
    *temp = *prog;
    temp++;
    *temp = '\0';
    prog++;
    token_type = BLOCK;
  }
  else
  {
    // Is or might be a relation operator
    if(strchr("!<>=+-*/%&|", *prog))
    {
      char temp_tok = '\0';
      switch(*prog)
      {
        // *******************************************************************
        case '=':
          if(*(prog + 1) == '=') temp_tok = EQ;
          break;
        // *******************************************************************
        case '!':
          if(*(prog + 1) == '=') temp_tok = NE;
          break;
        // *******************************************************************
        case '<':
          if(*(prog + 1) == '=')      temp_tok = LE;
          else if(*(prog + 1) == '<') temp_tok = LS;
          else
          {
            // Single character token processed there
            prog++; *temp = LT; temp++; *temp = '\0';
            token_type = DELIMITER;
          }
          break;
        // *******************************************************************
        case '>':
          if(*(prog + 1) == '=')      temp_tok = GE;
          else if(*(prog + 1) == '>') temp_tok = RS;
          else
          {
            // Single character token processed there
            prog++; *temp = GT; temp++; *temp = '\0';
            token_type = DELIMITER;
          }
          break;
        // *******************************************************************
        case '+':
          if(*(prog + 1) == '+') temp_tok = INC;
          if(*(prog + 1) == '=') temp_tok = ADD;
          break;
        // *******************************************************************
        case '-':
          if(*(prog + 1) == '-') temp_tok = DEC;
          if(*(prog + 1) == '=') temp_tok = SUB;
          break;
        // *******************************************************************
        case '*':
          if(*(prog + 1) == '=') temp_tok = MUL;
          break;
        // *******************************************************************
        case '/':
          if(*(prog + 1) == '=') temp_tok = DIV;
          break;
        // *******************************************************************
        case '%':
          if(*(prog + 1) == '=') temp_tok = MOD;
          break;
        // *******************************************************************
        case '&':
          if(*(prog + 1) == '&') temp_tok = AND;
          break;
        // *******************************************************************
        case '|':
          if(*(prog + 1) == '|') temp_tok = OR;
          break;
      }
      // If we need to set two characters token
      if(temp_tok != '\0')
      {
        // Advance prog two characters
        prog++; prog++;
        // Set token
        *temp = temp_tok; temp++; *temp = temp_tok; temp++; *temp = '\0';
        // Set token type
        token_type = DELIMITER;
      }
    }

    if(token_type == UNDEFTT)
    {
      // Check for other delimiters
      if(strchr("!+-*^/%=;:(),'?", *prog))
      {
        *temp = *prog;
        prog++; // advance to next position
        temp++;
        *temp = '\0';
        token_type = DELIMITER;
      }
      else if(*prog=='"') // Quoted string
      {
        token_ptr = prog; // Save string pointer to use get_token() on it later
        prog++; // Pass "
        if((*prog == '\r') || (*prog == '\n') || (*prog == '\0')) result = sntx_err(SYNTAX);
        // Copy string
        while((*prog != '"') && result)
        {
          // Check for \n escape sequence and replace it with \n character
          if(*prog == '\\')
          {
            if(*(prog + 1) == 'n')
            {
              prog++;
              *temp++ = '\n';
            }
          }
          else if((temp - token) < (int)(sizeof(token)))
          {
            *temp++ = *prog;
          }
          else
          {
            result = sntx_err(TOO_LONG_TOKEN);
            break;
          }
          // Advance program pointer
          prog++;
          // Check character
          if((*prog == '\r') || (*prog == '\n') || (*prog == '\0')) result = sntx_err(SYNTAX);
        }
        prog++; // Pass after "
        *temp = '\0';
        token_type = STRING;
      }
      else if(isdigit(*prog)) // Number
      {
        while(!isdelim(*prog)) *temp++ = *prog++;
        *temp = '\0';
        token_type = NUMBER;
      }
      else
      {
        if(isalpha(*prog)) // Var or command
        {
          // Save token pointer. Valid only for variable, command, function or string.
          token_ptr = prog;
          while(!isdelim(*prog)) *temp++ = *prog++;
          token_type = TEMP;
        }

        *temp = '\0';

        // See if a string is a command or a variable
        if(token_type == TEMP)
        {
          tok = look_up(token); // convert to internal rep
          if(tok) token_type = KEYWORD; // is a keyword
          else token_type = IDENTIFIER;
        }
      }
    }
  }

  if(token_type == UNDEFTT)
  {
    // Undefined token - end of program
    *token = '\0';
    tok = END;
    token_type = DELIMITER;
    // Show an error
    result = sntx_err(UNDEFINED_TOKEN);
  }

  return result;
}
// *****************************************************************************
// ***   Get string token by index   *******************************************
// *****************************************************************************
bool LittleC::get_string_token(int idx)
{
  const char* tmp = prog; // Save current program idx
  prog = &p_buf[idx]; // Set prog to string token
  bool result = get_token(); // Get token to fill token[]
  if(result && (token_type != STRING)) result = sntx_err(NOT_STRING);
  prog = tmp; // Restore prog index. Last token is lost, putback can't be used after that, only get_token can be used.
  return result;
}

// *****************************************************************************
// ***   Return a token to input stream   **************************************
// *****************************************************************************
void LittleC::putback(void)
{
  char *t = token;
  for(; *t; t++) prog--;
}

// *****************************************************************************
// ***   Look up a token's internal representation in the token table   ********
// *****************************************************************************
int LittleC::look_up(char *s)
{
// Conversation to lower case should be done only for internal commands.
// For variables and function names we want to have them unchanged. 
// For now this converation is disabled, right way to do it to copy string to
// temporary buffer and convert it to lower case.
// 
//  // Convert to lowercase
//  char *p = s;
//  while(*p) {*p = tolower(*p); p++;}

  // See if token is in table
  for(register int i = 0; *table[i].command; i++)
  {
    if(!strcmp(table[i].command, s)) return table[i].tok;
  }

  return 0; // unknown command
}

// *****************************************************************************
// ***   Return index of internal library function or -1 if not found   ********
// *****************************************************************************
int LittleC::internal_func(char *s)
{
  for(int i = 0; intern_func[i].f_name[0]; i++)
  {
    if(!strcmp(intern_func[i].f_name, s)) return i;
  }
  return -1;
}

// *****************************************************************************
// ***   Return true if c is a delimiter   *************************************
// *****************************************************************************
int LittleC::isdelim(char c)
{
  if(strchr(" !:;,+-<>'/*%^=()?", c) || (c == 9) || (c == '\r') || (c == '\n') || (c == 0)) return 1;
  return 0;
}

// *****************************************************************************
// ***   Return 1 if c is space or tab   ***************************************
// *****************************************************************************
int LittleC::iswhite(char c)
{
  if((c == ' ') || (c == '\t')) return 1;
  else return 0;
}

// *****************************************************************************
// ***   Compare two strings, return 0 if equal   ******************************
// *****************************************************************************
bool LittleC::strcomp(const char* str1, const char* str2)
{
  bool is_different = false;

  // Cycle until both delimiter found
  while(!(isdelim(*str1) && isdelim(*str2)))
  {
    // Strings can have different delimiters
    if(*str1 != *str2)
    {
      is_different = true;
      break;
    }
    // increase pointers to move to next character
    str1++;
    str2++;
  }

  return is_different;
}

// *****************************************************************************
// *****************************************************************************
// ***   LCLIB.cpp   ***********************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Helper function to avoid copy-paste for functions without arguments   *
// *****************************************************************************
bool LittleC::no_arg_func()
{
  bool result = get_token();
  if(result && (*token != '(')) result = sntx_err(PAREN_EXPECTED);
  if(result) result = get_token();
  if(result && (*token != ')')) result = sntx_err(PAREN_EXPECTED);
  return result;
}

// *****************************************************************************
// ***   Put a character to the display   **************************************
// *****************************************************************************
bool LittleC::call_putch(data_type& ret)
{
  bool result = eval_exp(ret);

  if(result && (p_output != nullptr) && (cur_pos < (output_size - 1)))
  {
    p_output[cur_pos] = ret.value;
    cur_pos++;
  }
  else
  {
    result = false;
  }

  return result;
}

// *****************************************************************************
// ***   Call puts()   *********************************************************
// *****************************************************************************
bool LittleC::call_puts(data_type& ret)
{
  bool result = true;

  ret = {0};

  get_token();
  if(*token != '(') result = sntx_err(PAREN_EXPECTED);
  if(result)
  {
    get_token();
    if(token_type != STRING) result = sntx_err(QUOTE_EXPECTED);
  }
  if(result)
  {
    // Display error and line number
    snprintf(&p_output[cur_pos], output_size - cur_pos, "%s\n", token);
    // Move current position to the end
    while((p_output[cur_pos] != '\0') && (cur_pos < output_size)) cur_pos++;

    get_token();
    if(*token != ')') result = sntx_err(PAREN_EXPECTED);
  }

  return result;
}

// *****************************************************************************
// ***   A built-in console output function   **********************************
// *****************************************************************************
bool LittleC::call_print(data_type& ret)
{
  bool result = true;

  ret = {VOID, 0};

  get_token();
  if(*token != '(') result = sntx_err(PAREN_EXPECTED);
  else *token = ','; // Set token to enter inside next cycle

  // Do cycle to process all arguments
  while((*token != ')') && (*token == ',') && result)
  {
    get_token();

    // No arguments call - just exit
    if(*token == ')')
    {
      break;
    }
    else if(token_type == STRING) // Output a string
    {
      // Print string
      if(p_output != nullptr) snprintf(&p_output[cur_pos], output_size - cur_pos, token);
    }
    else // Output result of expression
    {
      // putback last token to process expression
      putback();
      // Process expression
      data_type data = {0};
      result = eval_exp(data);
      // Output the result
      if((result) && (p_output != nullptr))
      {
        switch(data.type)
        {
          case VOID:
            // Void type - just ignore and output nothing
            break;
          case CHAR:
            snprintf(&p_output[cur_pos], output_size - cur_pos, "%c", (char)data.value);
            break;
          case INT:
            snprintf(&p_output[cur_pos], output_size - cur_pos, "%d", (int)data.value);
            break;
          case STRING:
            result = get_string_token(data.value);
            if(result) snprintf(&p_output[cur_pos], output_size - cur_pos, token);
            break;
          default:
            // Any other type - error
            result = sntx_err(TYPE_EXPECTED);
            break;
        }
      }
    }
    // Move current position to the end
    if(p_output != nullptr) while((p_output[cur_pos] != '\0') && (cur_pos < output_size)) cur_pos++;

    get_token();
  }

  if(result && (*token != ')')) result = sntx_err(PAREN_EXPECTED);

  return result;
}

// *****************************************************************************
// ***   A built-in console output function with new line   ********************
// *****************************************************************************
bool LittleC::call_println(data_type& ret)
{
  bool result = call_print(ret);

  if(result && (p_output != nullptr) && (cur_pos < (output_size - 1)))
  {
    p_output[cur_pos++] = '\n'; // add new line character
    p_output[cur_pos] = '\0';   // and null-terminate string
  }
  else
  {
    result = false;
  }

  return result;
}

// *****************************************************************************
// ***   A built-in console output function for fixed-point number   ***********
// *****************************************************************************
bool LittleC::call_printfp(data_type& ret)
{
  bool result = true;
  data_type data = {0}, scaler = {0};
  int precision = 0; // Variables to convert scaler to precision

  result = get_token();
  if(result && (*token != '(')) result = sntx_err(PAREN_EXPECTED);
  if(result) result = eval_exp(data);
  if(result) result = get_token();
  if(result && (*token != ',')) result = sntx_err(PARAM_ERR);
  if(result) result = eval_exp(scaler);
  if(result) result = get_token();
  if(result && (*token != ')')) result = sntx_err(PAREN_EXPECTED);

  // Convert scaler to precision to use later
  if(result)
  {
    // Save scaler value to mess with it
    int scl_val = scaler.value;
    // Convert scaler to precision
    while(scl_val >= 10)
    {
      // Scaler should be power of 10
      if(scl_val % 10)
      {
        break;
      }
      else
      {
        scl_val /= 10;
        precision++;
      }
    }
    // Scaler should be power of 10 and precision shouldn't be greater than 9
    if((scl_val != 1) || (precision > 9))
    {
      result = sntx_err(PARAM_ERR);
    }
  }

  // Print string
  if(result && (p_output != nullptr))
  {
    if(scaler.value == 1)
    {
      snprintf(&p_output[cur_pos], output_size - cur_pos, "%d", data.value);
    }
    else
    {
      // Format string. * should be replaced to number
      char fmt_str[] = "%s%d.%0*d";
      // Replace * with precision number
      fmt_str[7u] = 0x30 + precision;

      // Find sign: sigh should be handled separately, because it will be lost
      // for values less than scaler.
      bool is_negative = data.value < 0;
      // Remove sign from number
      data.value = abs(data.value);

      // Print string
      snprintf(&p_output[cur_pos], output_size - cur_pos, fmt_str, is_negative ? "-" : "", data.value / scaler.value, data.value % scaler.value);
    }
    // Move current position to the end
    while((p_output[cur_pos] != '\0') && (cur_pos < output_size)) cur_pos++;
  }

  // This function return VOID type which allow to use if inside print()
  ret.type = VOID;
  ret.value = 0;

  return result;
}

// *****************************************************************************
// ***   Return absolute value   ***********************************************
// *****************************************************************************
bool LittleC::call_abs(data_type& ret)
{
  bool result = eval_exp(ret);
  if(result) ret.value = abs(ret.value);
  return result;
}

// *****************************************************************************
// ***   Return current X axis value   *****************************************
// *****************************************************************************
bool LittleC::call_getaxisposx(data_type& ret)
{
  ret.type = INT;
  ret.value = GrblComm::GetInstance().GetAxisPosition(GrblComm::AXIS_X);
  return no_arg_func();
}

// *****************************************************************************
// ***   Return current Y axis value   *****************************************
// *****************************************************************************
bool LittleC::call_getaxisposy(data_type& ret)
{
  ret.type = INT;
  ret.value = GrblComm::GetInstance().GetAxisPosition(GrblComm::AXIS_Y);
  return no_arg_func();
}

// *****************************************************************************
// ***   Return current Z axis value   *****************************************
// *****************************************************************************
bool LittleC::call_getaxisposz(data_type& ret)
{
  ret.type = INT;
  ret.value = GrblComm::GetInstance().GetAxisPosition(GrblComm::AXIS_Z);
  return no_arg_func();
}

// *****************************************************************************
// ***   Return current mode(radius/diameter)   ********************************
// *****************************************************************************
bool LittleC::call_islathediametermode(data_type& ret)
{
  ret.type = INT;
  ret.value = GrblComm::GetInstance().IsLatheDiameterMode();
  return no_arg_func();
}
