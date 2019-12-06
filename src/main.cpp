#include <fstream>
#include <iostream>
#include <string_view>
#include <cstring>
#include "../headers/tree.h"

using std::string_view;

const int MAX_SINGLE_WORD_LENGTH = 256;
const int MAX_VARIABLE_LENGTH = 256;
const int MAX_FUNCTION_BUFFER = 8192;

struct Function {
    string_view name;
    string_view *var_list;
    string_view block;
    size_t n_args;
};

string_view *splitToFunctions (char *source, size_t *number_of_functions)
{
  char *source_start = source;
  size_t read_symbols = 0;
  *number_of_functions = 0;
  char *current_word_buffer = new char[MAX_SINGLE_WORD_LENGTH] ();

  while (*source)
    {
      sscanf (source, "%s%n", current_word_buffer, &read_symbols);
      source += read_symbols;

      if (strcmp (current_word_buffer, "slave") == 0)
        {
          (*number_of_functions)++;
        }
    }

  source = source_start;
  string_view *functions = new string_view[*number_of_functions] ();
  char *previous_function_start = source_start;
  size_t n_extracted_functions = 0;

  while (*source)
    {
      sscanf (source, "%s%n", current_word_buffer, &read_symbols);

      if (strcmp (current_word_buffer, "slave") == 0)
        {
          if (previous_function_start != source_start)
            {
              functions[n_extracted_functions] = string_view (previous_function_start,
                                                              source - previous_function_start);
              n_extracted_functions++;
            }
          previous_function_start = source + read_symbols + 1;
        }
      source += read_symbols;
    }
  if (previous_function_start != source_start && n_extracted_functions < *number_of_functions)
    {
      functions[*number_of_functions - 1] = string_view (previous_function_start, source - previous_function_start);
    }
  return functions;
}

size_t countArgs (char *buffer)
{
  size_t n_args = 0;
  while (*buffer != ')')
    {
      if (*buffer != ',' && *buffer != '(' && *buffer != ' ')
        {
          n_args++;
          while (*buffer != ',' && *buffer != '(' && *buffer != ' ' && *buffer != ')') buffer++;
        }
      else
        {
          buffer++;
        }
    }
  return n_args;
}

string_view *getArgs (char *buffer)
{
  size_t n_args = countArgs (buffer);

  if(!n_args) return nullptr;

  string_view* args = new string_view[n_args]();
  while(*buffer == ' ' || *buffer == '(') buffer++;
  size_t symbols_read = 0;
  if(n_args == 1)
    {
      sscanf(buffer, "%*[^)]%n", &symbols_read);
      args[0] = string_view (buffer, symbols_read);
      return args;
    }
    int i = 0;
  while (*buffer != ')')
    {
      if(n_args > 1)
        {
          sscanf (buffer, "%*[^,]%n", &symbols_read);
        }
       else sscanf (buffer, "%*[^)]%n", &symbols_read);
      args[i] = string_view (buffer, symbols_read);
      buffer += symbols_read;
      while(*buffer == ' ' || *buffer == ',') buffer++;
      i++;
      n_args--;
    }
  return args;
}

string_view getBlock(string_view* function)
{
  size_t block_start = function->find("join_this_world\n");
  string_view function_body = function->substr(block_start + strlen("join_this_world\n"), function->size() - block_start - strlen("join_this_world") - strlen("\nend_life"));
  return function_body;
}

Function *buildFunctionStructs (string_view *functions, size_t n_functions)
{
  Function *function_structs = new Function[n_functions];
  for (int i = 0; i < n_functions; i++)
    {
      size_t name_length = 0;
      sscanf (functions[i].data (), "%*s%n", &name_length);
      function_structs[i].name = string_view (functions[i].data (), name_length);
      functions[i].remove_prefix(name_length + 1);

      function_structs[i].var_list = getArgs ((char*) functions[i].data());
      function_structs[i].n_args = countArgs ((char*) functions[i].data());
      function_structs[i].block = getBlock (&functions[i]);
    }
  return function_structs;
}

int main ()
{
  FILE *input = fopen ("example.mhead", "r");
  fseek (input, 0, SEEK_END);
  size_t file_size = ftell (input);
  rewind (input);
  char *data = new char[file_size + 1];

  fread (data, sizeof (char), file_size, input);
  fclose (input);

  size_t number_of_functions = 0;
  string_view *functions = splitToFunctions (data, &number_of_functions);
  Function *function_structs = buildFunctionStructs (functions, number_of_functions);

  std::cout << "Name: " << function_structs[0].name << std::endl;
  std::cout << "Number of arguments: " << function_structs[0].n_args << std::endl;
  std::cout << "Args: ";
  for(int i = 0 ; i < function_structs[0].n_args; i++)
    {
      std::cout << function_structs[0].var_list[i] << " ";
    }
 std::cout << std::endl;

  std::cout << "Body: " << std::endl;
  std::cout << function_structs[0].block;

  return 0;
}
