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
    else
      {
        return GetVariable ();
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
