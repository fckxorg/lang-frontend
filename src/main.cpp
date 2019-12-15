#include <fstream>
#include <iostream>
#include <string_view>
#include <cstring>

#include "../headers/config.h"
#include "../headers/tree.h"
#include "../headers/TreeBuilder.h"

using std::string_view;

struct Function {
    string_view name;
    string_view *var_list;
    string_view *block;
    size_t n_args;
};

Node<string_view *> *parseBlock (string_view *block, size_t *start_position);
Node<string_view *> *parseLine (string_view *line, size_t *position);

string_view *splitToFunctions (char *source, size_t *number_of_functions)
{
  string_view source_code = string_view (source);
  size_t read_symbols = 0;
  *number_of_functions = 0;
  size_t start_position = 0;

  while (source_code.find ("slave", start_position) != string_view::npos)
    {
      (*number_of_functions)++;
      start_position += source_code.find ("slave", start_position) + 6;
    }
  auto *functions = new string_view[*number_of_functions] ();

  size_t function_start = 0;
  size_t function_end = 0;
  size_t n_extracted_functions = 0;
  start_position = 0;

  while (source_code.find ("slave", start_position) != string_view::npos
         && n_extracted_functions < *number_of_functions - 1)
    {
      function_start = source_code.find ("slave", start_position) + 6;
      start_position = function_start;
      function_end = source_code.find ("slave", start_position);
      functions[n_extracted_functions] = source_code.substr (function_start, function_end - function_start);
      n_extracted_functions++;
    }
  function_start = source_code.find ("slave", start_position) + 6;
  function_end = source_code.size ();
  functions[*number_of_functions - 1] = source_code.substr (function_start, function_end - 1 - function_start);
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
      else buffer++;
    }
  return n_args;
}

string_view *getArgs (char *buffer)
{
  size_t n_args = countArgs (buffer);

  if (!n_args) return nullptr;

  auto *args = new string_view[n_args] ();
  while (*buffer == ' ' || *buffer == '(') buffer++;
  size_t symbols_read = 0;
  if (n_args == 1)
    {
      sscanf (buffer, "%*[^)]%n", &symbols_read);
      args[0] = string_view (buffer, symbols_read);
      return args;
    }
  int i = 0;
  while (*buffer != ')')
    {
      if (n_args > 1)
        {
          sscanf (buffer, "%*[^,]%n", &symbols_read);
        }
      else sscanf (buffer, "%*[^)]%n", &symbols_read);
      args[i] = string_view (buffer, symbols_read);
      buffer += symbols_read;
      while (*buffer == ' ' || *buffer == ',') buffer++;
      i++;
      n_args--;
    }
  return args;
}

string_view *getFunctionBody (string_view *function)
{
  size_t block_start = function->find ("join_this_world\n") + strlen ("join_this_world");
  while (!isalpha (*((char *) function->data () + block_start))) block_start++;
  size_t block_end = function->rfind ("end_life");
  auto *function_body = new string_view ();
  *function_body = function->substr (block_start, block_end - block_start);

  return function_body;
}

Function *buildFunctionStructs (string_view *functions, size_t n_functions)
{
  auto *function_structs = new Function[n_functions];
  for (
      int i = 0;
      i < n_functions;
      i++)
    {
      size_t name_length = 0;
      sscanf (functions[i]
                  .
                      data (),
              "%*s%n", &name_length);
      function_structs[i].
          name = string_view (functions[i].data (), name_length);
      functions[i].
          remove_prefix (name_length
                         + 1);

      function_structs[i].
          var_list = getArgs ((char *) functions[i].data ());
      function_structs[i].
          n_args = countArgs ((char *) functions[i].data ());
      function_structs[i].
          block = getFunctionBody (&functions[i]);
    }
  return
      function_structs;
}

Node<string_view *> *buildVarlistSubtree (string_view *varlist, size_t n_args)
{

  auto root_node = new Node<string_view *> (nullptr, 3);
  Node<string_view *> *current_root_node = root_node;
  for (
      int i = 0;
      i < n_args;
      i++)
    {
      auto id_node = new Node<string_view *> (&varlist[i], 4);
      auto new_varlist = new Node<string_view *> (nullptr, 3);
      current_root_node->
          right = id_node;
      id_node->
          parent = current_root_node;

      new_varlist->
          parent = current_root_node;
      current_root_node->
          left = new_varlist;

      current_root_node = new_varlist;
    }
  return
      root_node;
}

Node<string_view *> *buildSubtreeWithId (const char *command, string_view *line, int type, size_t *position)
{
  if (line->find (command, *position) != string_view::npos)
    {
      size_t id_start = line->find (command, *position) + strlen (command);
      size_t id_end = line->find (";", id_start);
      auto id = new string_view ();
      *id = line->substr (id_start, id_end - id_start);

      auto subtree_root = new Node<string_view *> (nullptr, type);
      subtree_root->right = new Node<string_view *> (id, ID);

      subtree_root->right->parent = subtree_root;
      *position += id_end - line->find (command, *position);
      while (!isalpha (*((char *) line->data () + *position))) (*position)++;
      return subtree_root;
    }
  else return nullptr;
}

Node<string_view *> *extractExpression (string_view *line, size_t expression_start, size_t expression_end)
{
  char exp_end_symbol = *(line->data () + expression_end);
  *((char *) line->data () + expression_end) = '\0';

  TreeBuilder builder;
  Tree<string_view *> *expression_tree = builder.build ((char *) line->data () + expression_start);
  *((char *) line->data () + expression_end) = exp_end_symbol;

  return expression_tree->root;
}

Node<string_view *> *parseExpression (string_view *line, size_t *position)
{
  size_t expression_start = line->find ("=", *position) + 2;
  size_t expression_end = line->find (";", *position);
  size_t id_end = line->find ("=", *position);
  while (!isalpha (*((char *) line->data () + id_end))) id_end--;
  size_t id_start = id_end;
  while (isalpha (*((char *) line->data () + id_start - 1))) id_start--;

  auto subtree_root = new Node<string_view *> (nullptr, ASSIGNMENT);
  subtree_root->left = new Node<string_view *> (nullptr, ID);
  subtree_root->left->parent = subtree_root;
  subtree_root->left->data = new string_view ();
  *(subtree_root->left->data) = line->substr (id_start, id_end - id_start + 1);

  *position += expression_end - id_start;
  while (!isalpha (*((char *) line->data () + *position))) (*position)++;

  subtree_root->right = extractExpression (line, expression_start, expression_end);
  subtree_root->right->parent = subtree_root;
  return subtree_root;

}

Node<string_view *> *parseBlock (string_view *block, size_t *position)
{
  auto *root = new Node<string_view *> (nullptr, BLOCK);
  Node<string_view *> *current = root;

  *position = block->find ("join_this_world", *position) + strlen ("join_this_world");
  while (!isalpha (*((char *) block->data () + *position))) (*position)++;

  while (true)
    {
      current->right = parseLine (block, position);
      if (current->right)
        {
          current->left = new Node<string_view *> (nullptr, OP);
          current->left->parent = current;
          current->right->parent = current;
          current = current->left;
        }
      else
        {
          current->parent->left = nullptr;
          delete current;
          return root;
        }
    }
}

Node<string_view *> *parseBlockInstruction (string_view *line, size_t *position, bool cycle)
{
  size_t start = 0;
  auto subtree_root = new Node<string_view *> (nullptr, 0);
  if (cycle) subtree_root->type = WHILE;
  else subtree_root->type = IF;

  size_t condition_start = line->find ("(", *position) + 1;
  size_t condition_end = line->find (")", *position);

  size_t equal_pos = line->find ("==", *position);
  size_t above_pos = line->find (">", *position);

  size_t min_pos = std::min (equal_pos, above_pos);

  if (min_pos == equal_pos) subtree_root->left = new Node<string_view *> (nullptr, EQUAL);
  if (min_pos == above_pos) subtree_root->left = new Node<string_view *> (nullptr, ABOVE);

  subtree_root->left->parent = subtree_root;

  subtree_root->left->left = extractExpression (line, condition_start, min_pos - 1);
  subtree_root->left->left->parent = subtree_root->left;

  subtree_root->left->right = extractExpression (line, min_pos + 3, condition_end);
  subtree_root->left->right->parent = subtree_root->left;

  *position = condition_end + 1;
  while (!isalpha (*(line->data () + *position))) (*position)++;

  subtree_root->right = parseBlock (line, position);
  subtree_root->right->parent = subtree_root;

  return subtree_root;
}

Node<string_view *> *parseLine (string_view *line, size_t *position)
{
  size_t return_pos = line->find ("i_wish_for_death", *position);
  size_t init_pos = line->find ("new_blood", *position);
  size_t input_pos = line->find ("pray_to_God", *position);
  size_t output_pos = line->find ("God_take", *position);
  size_t exp_pos = line->find ("=", *position);
  size_t if_pos = line->find ("hope_that", *position);
  size_t while_pos = line->find ("nothing_could_stop_me_but", *position);
  size_t block_end = line->find ("end_life", *position);

  size_t min_value = std::min (std::min (std::min (while_pos, block_end), std::min (return_pos, init_pos)), std::min (std::min (input_pos, output_pos), std::min (exp_pos, if_pos)));

  if (min_value == return_pos) return buildSubtreeWithId ("i_wish_for_death", line, RETURN, position);
  if (min_value == init_pos) return buildSubtreeWithId ("new_blood", line, INITIALIZE, position);
  if (min_value == input_pos) return buildSubtreeWithId ("pray_to_God", line, INPUT, position);
  if (min_value == output_pos) return buildSubtreeWithId ("God_take", line, OUTPUT, position);
  if (min_value == exp_pos) return parseExpression (line, position);

  if (min_value == if_pos) return parseBlockInstruction (line, position, false);
  if (min_value == while_pos) return parseBlockInstruction (line, position, true);
  if (min_value == block_end)
    {
      *position = block_end + strlen ("end_life");
      while (!isalpha (*(line->data () + *position))) (*position)++;
      return nullptr;
    }
}

Node<string_view *> *parseFunctionBody (string_view *body)
{
  size_t position = 0;
  auto *root = new Node<string_view *> (nullptr, BLOCK);
  Node<string_view *> *current = root;

  while (position < body->size ())
    {
      current->right = parseLine (body, &position);
      if (current->right)
        {
          current->left = new Node<string_view *> (nullptr, OP);
          current->left->parent = current;
          current->right->parent = current;
          current = current->left;
        }
    }
  current->parent->left = nullptr;
  delete current;
  return root;
}

Tree<string_view *> *buildFunctionsTree (Function *functions, size_t n_functions)
{
  auto *programm_tree = new Tree<string_view *> ();
  programm_tree->root = programm_tree->newNode (nullptr, PROGRAMM_ROOT);

  auto declaration_node = programm_tree->newNode (nullptr, DECLARATION);
  programm_tree->connectNodeRight (programm_tree->getRoot (), declaration_node);

  for (int i = 0; i < n_functions; i++)
    {
      auto function_node = programm_tree->newNode (nullptr, FUNCTION);
      programm_tree->connectNodeRight (declaration_node, function_node);

      auto function_name = programm_tree->newNode (&functions[i].name, ID);
      programm_tree->connectNodeRight (function_node, function_name);

      auto varlist_node = buildVarlistSubtree (functions[i].var_list, functions[i].n_args);
      programm_tree->connectNodeLeft (function_node, varlist_node);

      if (i != n_functions - 1)
        {
          auto new_declaration_node = programm_tree->newNode (nullptr, DECLARATION);
          programm_tree->connectNodeLeft (declaration_node, new_declaration_node);
          declaration_node = new_declaration_node;
        }

      size_t start_position = 0;
      auto block_node = parseFunctionBody (functions[i].block);
      programm_tree->connectNodeRight (function_name, block_node);

    }
  return programm_tree;
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

  auto tree = buildFunctionsTree (function_structs, number_of_functions);
  tree->dump ("../dump.dot");
  system ("dot -Tpng ../dump.dot > ../dump.png");

  return 0;
}
