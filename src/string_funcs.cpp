//
// Created by maxim on 06.09.19.
//

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include "../headers/string_funcs.h"

int writeFileFromBuffer (char *filepath, char *buffer, int file_size)
{
  /*!Writes string to file located in provided path
   * @param filepath path to file location
   * @param buffer buffer to write
   * @param file_size length of file
   * */

  assert(filepath);
  assert(buffer);

  FILE *file = fopen (filepath, "a");

  if (!file)
    {
      perror ("Failed to open file");
      return 1;
    }
  fwrite (buffer, sizeof (char), file_size, file);

  return 0;
}

int writeFileFromIndex (char *filepath, StringBoundaries *index, int n_lines)
{
  /*!Writes strings to file located in provided path
   * @param filepath path to file location
   * @param strings_start array of pointers to strings to write
   * @param n_lines number of lines in file
   * */
  assert(filepath);
  assert(index);

  FILE *file = fopen (filepath, "a");

  if (!file)
    {
      perror ("Failed to open file");
      return 1;
    }

  for (int i = 0; i < n_lines; i++)
    {
      fprintf (file, "%s\n", index[i].start);
    }
  fprintf (file, "\n");
  fclose (file);

  return 0;
}

// Completed
int getFileSize (char *filepath)
{
  /*!Returns file length in symbols, writes number of lines
   * @param filepath path to file location
   * @param n_lines pointer to
   * @return file_size length of file in symbols
   * */
  assert(filepath);

  FILE *file = fopen (filepath, "r");

  if (!file)
    {
      perror ("Failed to open file");
      return -1;
    }

  int file_size = 0;
  fseek (file, 0, SEEK_END);
  file_size = ftell (file);
  fclose (file);

  return file_size;
}

int getNumberOfLines (char *file_data)
{
  /*! Use this function to get number of lines in buffer
   * @param file_data pointer to buffer
   * @return n_lines number of lines in buffer
   * */

  assert(file_data);

  int n_lines = 0;

  while (*file_data)
    {
      if (*file_data == '\n')
        {
          n_lines++;
        }
      file_data++;
    }

  return n_lines;
}

// Completed
int readFile (char *filepath, int file_size, char *file_data)
{
  /*!Writes file data to array
   * @param filepath path to file location
   * @param file_size length of file in symbols
   * @param file_data array for file data storing
   * */
  assert(filepath);
  assert (file_data);

  FILE *file = fopen (filepath, "r");

  if (!file)
    {
      perror ("Failed to open file");
      return 1;
    }

  fseek (file, 0, SEEK_SET);
  fread (file_data, sizeof (char), file_size, file);
  fclose (file);
  return 0;
}

void getStringsBoundaries (char *file_data, int file_size, StringBoundaries *index)
{
  /*!Writes list of pointers to string starts
   * @param file_data array with file_data
   * @param file_size length of file in symbols
   * @param string_starts array for pointers storing
   * */
  assert (file_data);
  assert (index);

  (*index).start = file_data;

  for (int i = 1; i < file_size; i++)
    {
      if (file_data[i - 1] == '\n')
        {
          file_data[i - 1] = '\0';
          (*index).end = &file_data[i - 2];
          index++;
          (*index).start = &file_data[i];
        }
    }

  file_data[file_size - 1] = '\0';
  (*index).end = &file_data[file_size - 2];
}


char *checkLetter (char *symbol, char *gatherer (char *letter))
{
  /*!Checks if symbol is letter.
   * If it is not, returns ptr to new symbol, got by provided gatherer.
   * @param symbol ptr to symbol for checking
   * @param gatherer function for getting new letter
   * @return symbol next letter, or provided symbol if it is already letter
   * */

  assert (symbol);
  assert (gatherer);

  if (!isalpha (*symbol))
    {
      symbol = gatherer (symbol);
    }
  return symbol;
}

char *getPreviousLetter (char *symbol)
{
  /*! Use this function to get previous letter in buffer.
  * @param symbol ptr to current symbol in buffer
  * @return symbol ptr to previous letter in buffer
  * */

  assert (symbol);

  do
    {
      symbol--;
    }
  while (!isalpha (*symbol) && *symbol);

  return symbol;
}

char *getNextLetter (char *symbol)
{
  /*! Use this function to get next letter in buffer.
   * @param symbol ptr to current symbol in buffer
   * @return symbol ptr to next letter in buffer
   * */

  assert (symbol);

  do
    {
      symbol++;
    }
  while (!isalpha (*symbol) && *symbol);

  return symbol;
}

int compareStrings (const void *first_string, const void *second_string)
{
  /*! Use this method to compare two strings
   * @param first_string pointer to first buffer with string
   * @param second_string pointer to second buffer with string
   * @return negative value, zero or positive value, when first buffer is lower, equal or greater than second_buffer respective
   * */
  assert(first_string);
  assert (second_string);

  char *arg1 = (*((StringBoundaries *) first_string)).start;
  char *arg2 = (*((StringBoundaries *) second_string)).start;

  assert (arg1);
  assert (arg2);

  arg1 = checkLetter (arg1, getNextLetter);
  arg2 = checkLetter (arg2, getNextLetter);

  while (*arg1 == *arg2 && *arg1)
    {
      arg1 = getNextLetter (arg1);
      arg2 = getNextLetter (arg2);
    }

  int result = toupper (*arg1) - toupper (*arg2);

  return result;
}

int compareStringsBackwards (const void *first_string, const void *second_string)
{
  /*! Use this method to compare two strings from end to start
  * @param first_string pointer to first buffer with string
  * @param second_string pointer to second buffer with string
  * @return negative value, zero or positive value, when first buffer is lower, equal or greater than second_buffer respective
  * */

  assert(first_string);
  assert (second_string);

  char *arg1 = (*((StringBoundaries *) first_string)).end;
  char *arg2 = (*((StringBoundaries *) second_string)).end;

  assert (arg1);
  assert (arg2);

  checkLetter (arg1, getPreviousLetter);
  checkLetter (arg2, getPreviousLetter);

  while (*arg1 == *arg2 && *arg1)
    {
      arg1 = getPreviousLetter (arg1);
      arg2 = getPreviousLetter (arg2);
    }

  int result = toupper (*arg1) - toupper (*arg2);

  return result;
}

void sortStrings (StringBoundaries *index, int n_lines)
{
  /*!Sorts strings by pointers in array of structs
   * @param index array of structs with pointers to string starts
   * @param n_lines number of lines in file
   * */
  assert (index);

  qsort (index, n_lines, sizeof (StringBoundaries), compareStrings);
}

void sortStringsBackwards (StringBoundaries *index, int n_lines)
{
  /*!Sorts strings from end to start by pointers in array of structs
   * @param index array of structs with pointers to string starts
   * @param n_lines number of lines in file
   * */

  assert (index);

  qsort (index, n_lines, sizeof (StringBoundaries), compareStringsBackwards);
}

File loadFile (char *filename)
{
  /*! This function creates structure for file, containing file_size, number of lines,
   * index of structures with strings boundaries, processed file data buffer and raw data buffer
   * @param filename pointer to char pointer with filename
   * @return loaded_file Structure with information about file
   * */

  File loaded_file {};
  loaded_file.size = getFileSize (filename);
  loaded_file.data = (char *) calloc (loaded_file.size + 1, sizeof (char));
  loaded_file.raw_data = (char *) calloc (loaded_file.size + 1, sizeof (char));

  readFile (filename, loaded_file.size, loaded_file.data);
  memcpy (loaded_file.raw_data, loaded_file.data, loaded_file.size * sizeof (char));

  loaded_file.n_lines = getNumberOfLines (loaded_file.data);

  loaded_file.index = (StringBoundaries *) calloc (loaded_file.n_lines, sizeof (StringBoundaries));

  getStringsBoundaries (loaded_file.data, loaded_file.size, loaded_file.index);

  return loaded_file;
}