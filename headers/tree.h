#ifndef TREE_H
#define TREE_H

#include <cstring>
#include <map>

#include "string_funcs.h"

std::map<int, char *> types = {{0,   "PROGRAM_ROOT"},
                               {1,   "DECLARATION"},
                               {2,   "FUNCTION"},
                               {3,   "VARLIST"},
                               {4,   "ID"},
                               {5,   "BLOCK"},
                               {6,   "IF"},
                               {7,   "WHILE"},
                               {8,   "OP"},
                               {9,   "EXPRESSION"},
                               {10,  "VAR"},
                               {11,  "RETURN"},
                               {12,  "INPUT"},
                               {13,  "OUTPUT"},
                               {14,  "INITIALIZE"},
                               {15,  "ASSIGNMENT"},
                               {16,  "NUMBER"},
                               {17,  "OPERATOR"},
                               {18,  "CALL"},
                               {19,  "C"},
                               {100, "ADD"},
                               {101, "SUB"},
                               {102, "MUL"},
                               {103, "DIV"},
                               {104, "SIN"},
                               {105, "COS"},
                               {106, "TAN"},
                               {107, "SQR"},
                               {200, "EQUAL"},
                               {201, "ABOVE"}};

template<class Node_T>
class Node {
 public:
  Node_T data;
  Node<Node_T> *left;
  Node<Node_T> *right;
  Node<Node_T> *parent;
  int type;

  Node (Node_T value, int node_type)
  {
    left = nullptr;
    right = nullptr;
    parent = nullptr;
    data = value;
    type = node_type;
  }
};

std::string_view serialize (Node<std::string_view *> *node)
{
  if (node)
    {
      if (node->data) return *(node->data);
      return std::string_view (types[node->type]);
    }
  return std::string_view ("@");
}

void skipSpaces (char **buffer)
{
  while (**buffer == ' ')
    {
      (*buffer)++;
    }

}

template<class T>
class Tree {
 private:
  size_t n_nodes = 0;

  Node<std::string_view *> *loadNode (char **buffer)
  {
    Node<std::string_view *> *node = nullptr;
    size_t n_symbols = 0;
    skipSpaces(buffer);
    if (**buffer == '{')
      {
        (*buffer)++;
        skipSpaces(buffer);
        sscanf(*buffer,  "%*s%n", &n_symbols);
        std::string_view* data = new std::string_view(*buffer, n_symbols);


        (*buffer) += n_symbols;
        (*buffer)++;
        skipSpaces (buffer);

        auto node = new Node<std::string_view*>(data, 0);
        if(*data == "@") node = nullptr;
        if(**buffer == '}')
          {
            (*buffer)++;
            return node;
          }

        node->left = loadNode(buffer);
        node->right = loadNode(buffer);

        if(node->right) node->right->parent = node;
        if(node->left) node->left->parent = node;
        skipSpaces(buffer);

        if(**buffer == '}')
          {
            (*buffer)++;
            return node;
          }
         return nullptr;
      }
    return nullptr;
  }

  void dumpSubTree (Node<T> *node, std::ofstream &dump_file)
  {
    dump_file << "node" << node << "[label=\"{{" << node << "}|{TYPE|" << types[node->type] << "}|{VALUE|"
              << serialize (node)
              << "}|{LEFT|" << node->left
              << "}|{RIGHT|" << node->right << "}|{PARENT|" << node->parent << "}}}\",shape=record];" << std::endl;
    if (node->parent)
      {
        dump_file << "node" << node->parent << " -> node" << node << ";" << std::endl;
      }
    if (node->left) dumpSubTree (node->left, dump_file);
    if (node->right) dumpSubTree (node->right, dump_file);
  }

  void writeSubTreeToFile (Node<T> *node, std::ofstream &file)
  {
    file << serialize (node);
    if (node)
      {
        if (node->left || node->right)
          {
            file << " { ";
            writeSubTreeToFile (node->left, file);
            file << " } { ";
            writeSubTreeToFile (node->right, file);
            file << " }";
          }
      }
  }

 public:
  Node<T> *root = nullptr;

  Node<T> *search (Node<T> *node, T data)
  {
    if (strcmp (node->data, data) == 0) return node;
    Node<T> *left_subtree_node;
    Node<T> *right_subtree_node;
    if (node->left) left_subtree_node = search (node->left, data);
    else left_subtree_node = nullptr;
    if (node->right) right_subtree_node = search (node->right, data);
    else right_subtree_node = nullptr;

    if (right_subtree_node) return right_subtree_node;
    if (left_subtree_node) return left_subtree_node;
    return nullptr;
  }

  void createRoot (const T value, int type)
  {
    root = new Node<T> (value, type);
    n_nodes++;
  }

  Node<T> *getRoot ()
  {
    return root;
  }

  Node<T> *newNode (const T value, int type)
  {
    return new Node<T> (value, type);
  }

  void connectNodeLeft (Node<T> *parent, Node<T> *child)
  {
    parent->left = child;
    child->parent = parent;
    n_nodes++;
  }

  void connectNodeRight (Node<T> *parent, Node<T> *child)
  {
    parent->right = child;
    child->parent = parent;
    n_nodes++;
  }

  void deleteSubTree (Node<T> *subTreeRoot)
  {
    assert (subTreeRoot);

    if (subTreeRoot->parent)
      {
        if (subTreeRoot == subTreeRoot->parent->right) subTreeRoot->parent->right = nullptr;
        else if (subTreeRoot == subTreeRoot->parent->left) subTreeRoot->parent->left = nullptr;
        else perror ("Parental link broken");
        return;
      }

    if (subTreeRoot->right) deleteSubTree (subTreeRoot->right);
    if (subTreeRoot->left) deleteSubTree (subTreeRoot->left);

    delete subTreeRoot;
  }

  void saveToFile (char *filename)
  {
    std::ofstream file;
    file.open (filename);
    file << "{";
    writeSubTreeToFile (root, file);
    file << "}";
    file.close ();
  }

  void loadFromFile (char *filename)
  {
    File file{};
    file = loadFile (filename);
    char *buffer = file.raw_data;

    root = loadNode (&buffer);
  }

  void dump (const char *filename)
  {

    std::ofstream dump_file;
    dump_file.open (filename);
    dump_file << "digraph{" << std::endl;
    dumpSubTree (root, dump_file);
    dump_file << "}" << std::endl;
    dump_file.close ();
  }

  size_t countNodes (Node<T> *node)
  {
    size_t n_left_subtree = 0;
    size_t n_right_subtree = 0;
    if (node->left) n_left_subtree = countNodes (node->left);
    if (node->right) n_right_subtree = countNodes (node->right);
    return 1 + n_left_subtree + n_right_subtree;
  }

  bool verificateTree ()
  {
    if (countNodes (root) == n_nodes) return true;
    return false;

  }

};
#endif