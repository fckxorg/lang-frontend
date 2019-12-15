//
// Created by maxim on 26.11.19.
//
#include <cassert>
#include <string_view>

using std::string_view;

class TreeBuilder {
 public:
  Node<string_view *> *root = new Node<string_view *> (nullptr, OPERATOR);
  char *buffer = nullptr;
  Tree<string_view *> *build (char *expression)
  {
    buffer = expression;
    Tree<string_view *> *tree = new Tree<string_view *> ();
    tree->root = GetEquation ();
    return tree;

  }

 private:
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

  Node<string_view*> *GetNumber ()
  {
    size_t pos = 0;
    sscanf (buffer, "%*d%n", &pos);
    Node<string_view*> *leaf_node = new Node<string_view *>(new string_view(buffer, pos), NUMBER);
    buffer += pos;
    while(*buffer == ' ') buffer++;
    return leaf_node;
  }

  Node<string_view*> *GetVariable ()
  {
    int id_size = 0;
    if (isalpha (*buffer))
      {
        sscanf(buffer, "%*s%n", &id_size);
        Node<string_view*> *leaf_node = new Node<string_view*> (new string_view(buffer, id_size), ID);
        buffer += id_size;
        while(*buffer == ' ') buffer++;
        return leaf_node;
      }
    else
      {
        return GetNumber ();
      }
  }

  Node<string_view *> *GetTrig ()
  {
    Node<string_view*>* new_node = nullptr;
    char function_name[5] = "";
    sscanf (buffer, "%3c", function_name);
    if (strcmp (function_name, "sin") == 0)
      {
        new_node = new Node<string_view *> (new string_view(buffer, 3), SIN);
        buffer += 3;
        new_node->right = GetParenthesis ();
        new_node->right->parent = new_node;
      }
    else if (strcmp (function_name, "cos") == 0)
      {
        new_node = new Node<string_view *> (new string_view(buffer, 3), COS);
        buffer += 3;
        new_node->right = GetParenthesis ();
        new_node->right->parent = new_node;
      }
    else if (strcmp (function_name, "tan") == 0)
      {
        new_node = new Node<string_view *> (new string_view(buffer, 3), TAN);
        buffer += 3;
        new_node->right = GetParenthesis ();
        new_node->right->parent = new_node;
      }
    else if (strcmp (function_name, "sqr") == 0)
      {
        new_node = new Node<string_view *> (new string_view(buffer, 3), SQR);
        buffer += 3;
        new_node->right = GetParenthesis ();
        new_node->right->parent = new_node;
      }
    else {
        char call_buffer[MAX_SINGLE_WORD_LENGTH] = "";
        size_t read_symbols = 0;
        sscanf(buffer, "%s%n", call_buffer, &read_symbols);
        if(strcmp(call_buffer, "do_my_dirty_work") == 0)
          {
            buffer += read_symbols;
            while(*buffer == ' ') buffer++;

            sscanf(buffer, "%*s%n", &read_symbols);
            new_node = new Node<string_view *>(nullptr, CALL);
            new_node->left = new Node<string_view *>(new string_view(buffer, read_symbols), ID);
            new_node->left->parent = new_node;
            buffer += read_symbols;
            new_node->right = new Node<string_view *> (nullptr, VARLIST);
            new_node->right->parent = new_node;
            Node<string_view *>* current = new_node->right;

            size_t n_args = countArgs(buffer);
            string_view* args = getArgs(buffer);
            for(int i = 0; i < n_args; i++)
              {
                current->right = new Node<string_view *> (args+i, ID);
                current->right->parent = current;

                if(i != n_args - 1)
                  {
                    current->left = new Node<string_view *> (nullptr, VARLIST);
                    current->left->parent = current;
                    current = current->left;
                  }
              }
            while(*buffer != ')') buffer++;
            buffer++;
            return new_node;
          }
        else
          {
            return GetVariable ();
          }
    }

    return new_node;
  }

  Node<string_view *> *GetParenthesis ()
  {
    Node<string_view *> *brackets_tree = nullptr;
    if (*buffer == '(')
      {
        buffer++;
        while(*buffer == ' ') buffer++;
        brackets_tree = GetAddSub ();
        assert(*buffer == ')');
        buffer++;
      }
    else brackets_tree = GetTrig();
    return brackets_tree;
  }

  Node<string_view *> *GetMulDiv ()
  {
    Node<string_view *> *left_subtree = GetParenthesis ();
    Node<string_view *> *new_root = nullptr;
    while (*buffer == '*' || *buffer == '/')
      {
        char *op = buffer;
        buffer++;
        while (*buffer == ' ') buffer++;

        Node<string_view *> *right_subtree = GetParenthesis ();

        if (*op == '*') new_root = new Node<string_view *> (new string_view (op, 1), MUL);
        else new_root = new Node<string_view *> (new string_view (op, 1), DIV);

        new_root->left = left_subtree;
        left_subtree->parent = new_root;

        new_root->right = right_subtree;
        right_subtree->parent = new_root;

        left_subtree = new_root;
      }

    return left_subtree;
  }

  Node<string_view *> *GetAddSub ()
  {
    Node<string_view *> *left_subtree = GetMulDiv ();
    Node<string_view *> *new_root = nullptr;
    while (*buffer == '+' || *buffer == '-')
      {
        char *op = buffer;
        buffer++;
        while (*buffer == ' ') buffer++;
        Node<string_view *> *right_subtree = GetMulDiv ();

        if (*op == '+') new_root = new Node<string_view *> (new string_view (op, 1), ADD);
        else new_root = new Node<string_view *> (new string_view (op, 1), SUB);

        new_root->left = left_subtree;
        left_subtree->parent = new_root;

        new_root->right = right_subtree;
        right_subtree->parent = new_root;

        left_subtree = new_root;
      }
    return left_subtree;
  }

  Node<string_view *> *GetEquation ()
  {
    root = GetAddSub ();
    assert(*buffer == '\0');
    return root;
  }
};
