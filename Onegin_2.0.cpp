#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define NAME_FILE_FROM_BY_DEFAULT "Eugene_Onegin.txt"
#define NAME_FILE_TO_BY_DEFAULT "Sorted_Eugene_Onegin.txt"
#define INT_POISON 0xDE

#define FREE(Pointer, Size) do {                      \
  if (Pointer) {                                      \
    char *Safe_Pointer = (char *)((void *)Pointer);   \
    for (size_t Safe_i = 0; Safe_i < Size; Safe_i++)  \
        Safe_Pointer[Safe_i] = (char)INT_POISON;      \
    free((void *)Pointer);                            \
    Pointer = NULL;                                   \
  }                                                   \
} while (0)

const size_t NUMBER_OF_LINES = 5700;

enum Errors
{
  ERROR_NOT             = 0,
  ERROR_FIRST_SORTING   = -1,
  ERROR_SECOND_SORTING  = -2,
  ERROR_THIRD_SORTING   = -3,
  ERROR_READING_FILE    = -4
};

struct Line
{
  const char *Str;
  size_t Len;
};

size_t Read_From_File_And_Formation_Index(const char *Name_File, Line Index[],
                                          char *Buffer_Out[], size_t *Buffer_Size);
size_t Formation_Index(char *Buffer, size_t Bytes_Number, Line Index[]);
int Print_To_File(const char *Name_File, const Line Index[],
                  const size_t N_Lines, const char *Mode);

void Bubble_Sort(void *Array, size_t Length, size_t Size_Elem,
                 int (*Compare_Function)(const void *Number1, const void *Number2));
void Switch_Places(void *Value1, void *Value2, size_t Size_Elem);

int Compare_Str_Up(const void *Object1, const void *Object2);
int My_String_Strcmp(const char Str_First[], const char Str_Second[]);
int Compare_Str_Up_Reverse(const void *Object1, const void *Object2);
int My_String_Strcmp_Reverse(const char Str_First[], const char Str_Second[],
                             size_t Length_First, size_t Length_Second);
void Skip_Non_Letters(const char Str[], size_t *i);

int main(int argc, char *argv[])
{
  if (argc < 3)
    printf("ERROR: The command-line arguments are incorrect\n\n");

  const char *Name_File_From = (argc >= 2) ? argv[1] : NAME_FILE_FROM_BY_DEFAULT;
  const char *Name_File_To   = (argc >= 3) ? argv[2] : NAME_FILE_TO_BY_DEFAULT;

  Line Index[NUMBER_OF_LINES] = {};
  Line Copy_Index[NUMBER_OF_LINES] = {};
  char *Buffer = NULL;
  size_t Buffer_Size = 0;

  const size_t N_Lines = Read_From_File_And_Formation_Index(Name_File_From, Index,
                                                            &Buffer, &Buffer_Size);
  if (N_Lines == 0)
  {
    printf("ERROR: Could not read from file '%s'\n", Name_File_From);
    return ERROR_READING_FILE;
  }

  for (size_t i = 0; i < N_Lines; i++)
    Copy_Index[i] = Index[i];

  Bubble_Sort(Index, N_Lines, sizeof(Line), &Compare_Str_Up);
  if (Print_To_File(Name_File_To, Index, N_Lines, "w") < 0)
  {
    printf("ERROR: First sorting");
    return ERROR_FIRST_SORTING;
  }

  qsort(Index, N_Lines, sizeof(Line), &Compare_Str_Up_Reverse);
  if (Print_To_File(Name_File_To, Index, N_Lines, "a") < 0)
  {
    printf("ERROR: Second sorting");
    return ERROR_SECOND_SORTING;
  }

  if (Print_To_File(Name_File_To, Copy_Index, N_Lines, "a") < 0)
  {
    printf("ERROR: Third sorting");
    return ERROR_THIRD_SORTING;
  }

  FREE(Buffer, Buffer_Size);

  printf("Successful completion of the program\n\n");
  return ERROR_NOT;
}

size_t Read_From_File_And_Formation_Index(const char *Name_File, Line Index[],
                                          char *Buffer_Out[], size_t *Buffer_Size)
{
  assert(Name_File);
  assert(Index);
  assert(Buffer_Out);
  assert(Buffer_Size);

  struct stat Stats;
  if (stat(Name_File, &Stats) < 0)
    return 0;

  size_t File_Size = (size_t)(Stats.st_size);

  char *Buffer = (char *)calloc(File_Size + 2, sizeof(char));
  if (Buffer == NULL)
    return 0;

  int Descriptor = open(Name_File, O_RDONLY);
  if (Descriptor < 0)
  {
    FREE(Buffer, File_Size + 2);
    return 0;
  }

  ssize_t Bytes_Number = read(Descriptor, Buffer, (unsigned int)File_Size);
  close(Descriptor);

  if (Bytes_Number <= 0)
  {
    FREE(Buffer, File_Size + 2);
    return 0;
  }

  Buffer[Bytes_Number] = '\0';
  
  size_t N_Lines = Formation_Index(Buffer, (size_t)Bytes_Number, Index);

  *Buffer_Out = Buffer;
  *Buffer_Size = File_Size + 2;

  return N_Lines;
}

size_t Formation_Index(char *Buffer, size_t Bytes_Number, Line Index[])
{
  assert(Buffer);
  assert(Index);

  size_t N_Lines = 0, Max_Number_Of_Lines = 0, Count = 0;
  const char *Line_Start = Buffer;

  for (size_t i = 0; i < Bytes_Number; i++)
  {
    if (Buffer[i] == '\n')
    {
      if (Max_Number_Of_Lines < Count)
        Max_Number_Of_Lines = Count;
      Count = 0;
      Buffer[i] = '\0';

      Index[N_Lines].Str = Line_Start;
      Index[N_Lines].Len = (size_t)(&Buffer[i] - Line_Start);
      N_Lines++;

      Line_Start = &Buffer[i + 1];
    }
    else
    {
      Count++;
    }
  }

  if (Line_Start < Buffer + Bytes_Number)
  {
    Index[N_Lines].Str = Line_Start;
    Index[N_Lines].Len = (size_t)(Buffer + Bytes_Number - Line_Start);
    N_Lines++;
  }

  return N_Lines;
}

int Print_To_File(const char *Name_File, const Line Index[],
                  const size_t N_Lines, const char *Mode)
{
  assert(Name_File);
  assert(Index);
  assert(Mode);

  FILE * File = fopen(Name_File, Mode);
  if (File == NULL)
  {
    perror(Name_File);
    printf ("errno = %d\n", errno);
    return -1;
  }

  for (size_t i = 0; i < N_Lines; i++)
  {
    if (Index[i].Len == 0)
      continue;

    if (fwrite(Index[i].Str, sizeof(char), Index[i].Len, File) != Index[i].Len)
    {
      perror("fwrite could not write to the file");
      printf ("errno = %d\n", errno);
      if (fclose(File) != 0)
        printf("WARNING: File didn't close successfully");
      return -1;
    }

    if (fputc('\n', File) == EOF)
    {
      perror("fputc could not write to the file");
      printf ("errno = %d\n", errno);
      if (fclose(File) != 0)
        printf("WARNING: File didn't close successfully");
      return -1;
    }
  }

  fprintf(File, "\n\n\n--------------------------------END--------------------------------\n\n\n");
  if (fclose(File) != 0)
    printf("WARNING: File didn't close successfully");

  return 0;
}

void Bubble_Sort(void *Array, size_t Length, size_t Size_Elem,
                 int (*Compare_Function)(const void *Number1, const void *Number2))
{
  assert(Array);
  assert(Length > 0);
  assert(Size_Elem > 0);
  assert(*Compare_Function);

  void *Elem1 = NULL, *Elem2 = NULL;

  for (size_t Pass_Number = 0; Pass_Number < Length - 1; Pass_Number++)
  {
    bool Already_Sorted = true;

    for (size_t Element_Number = 0; Element_Number < Length - 1 - Pass_Number; Element_Number++)
    {
      Elem1 = (void *)((size_t)Array + Element_Number * Size_Elem);
      Elem2 = (void *)((size_t)Array + (Element_Number + 1) * Size_Elem);

      if ((*Compare_Function)(Elem1, Elem2) > 0)
      {
        Switch_Places(Elem1, Elem2, Size_Elem);
        Already_Sorted = false;
      }
    }

    if (Already_Sorted)
      break;
  }

  return;
}

void Switch_Places(void *Value1, void *Value2, size_t Size_Elem)
{
  assert(Value1);
  assert(Value2);
  assert(Size_Elem > 0);

  char Temp = 0;
  for (size_t i = 0; i < Size_Elem; i++)
  {
    Temp = *(char *)((size_t)Value1 + i);
    *(char *)((size_t)Value1 + i) = *(char *)((size_t)Value2 + i);
    *(char *)((size_t)Value2 + i) = Temp;
  }

  return;
}

int Compare_Str_Up(const void *Object1, const void *Object2)
{
  assert(Object1);
  assert(Object2);

  const Line *Line1 = (const Line *)Object1;
  const Line *Line2 = (const Line *)Object2;

  return My_String_Strcmp(Line1->Str, Line2->Str);
}

int My_String_Strcmp(const char Str_First[], const char Str_Second[])
{
  assert(Str_First);
  assert(Str_Second);

  if (Str_First[0] == '\0' || Str_Second[0] == '\0')
  {
    if (Str_First[0] == '\0' && Str_Second[0] == '\0')
      return 0;
    return (Str_First[0] == '\0') ? -((unsigned char)(Str_Second[0])) :
                                      (unsigned char)(Str_First[0]);
  }

  size_t i_First = 0, i_Second = 0;

  for (; Str_First[i_First + 1] != '\0' && Str_Second[i_Second + 1] != '\0'; i_First++, i_Second++)
  {
    Skip_Non_Letters(Str_First, &i_First);
    Skip_Non_Letters(Str_Second, &i_Second);

    if (Str_First[i_First] == '\0' && Str_Second[i_Second] == '\0')
      break;

    int Letter1 = tolower(Str_First[i_First]);
    int Letter2 = tolower(Str_Second[i_Second]);

    if (Letter1 == Letter2)
    {
      if (isblank(Str_First[i_First + 1]) && isalpha(Str_Second[i_Second + 1]))
        return -((unsigned char)(tolower(Str_Second[i_Second + 1])));

      else if (isalpha(Str_First[i_First + 1]) && isblank(Str_Second[i_Second + 1]))
        return (unsigned char)(tolower(Str_First[i_First + 1]));

      else if (Str_First[i_First + 1] == '\0' && Str_Second[i_Second + 1] == '\0')
        break;

      else
        continue;
    }

    else
      return (unsigned char)Letter1 - (unsigned char)Letter2;
  }

  return 0;
}

int Compare_Str_Up_Reverse(const void *Object1, const void *Object2)
{
  assert(Object1);
  assert(Object2);

  const Line *Line1 = (const Line *)Object1;
  const Line *Line2 = (const Line *)Object2;

  return My_String_Strcmp_Reverse(Line1->Str, Line2->Str, Line1->Len, Line2->Len);
}

int My_String_Strcmp_Reverse(const char Str_First[], const char Str_Second[],
                             size_t Length_First, size_t Length_Second)
{
  assert(Str_First);
  assert(Str_Second);
  assert(Length_First >= 0);
  assert(Length_Second >= 0);

  if (Str_First[0] == '\0' || Str_Second[0] == '\0')
  {
    if (Str_First[0] == '\0' && Str_Second[0] == '\0')
      return 0;
    return (Str_First[0] == '\0') ? -((unsigned char)(Str_Second[0])) :
                                      (unsigned char)(Str_First[0]);
  }

  size_t i_First = Length_First - 1, i_Second = Length_Second - 1;
  for (; (i_First > 0) && (i_Second > 0); i_First--, i_Second--)
  {
    while ((i_First > 0) && !isalpha(Str_First[i_First]))
      i_First--;

    while ((i_Second > 0) && !isalpha(Str_Second[i_Second]))
      i_Second--;

    if (!i_First && !i_Second)
      break;

    int Letter1 = tolower(Str_First[i_First]);
    int Letter2 = tolower(Str_Second[i_Second]);

    if (Letter1 == Letter2)
    {
      if (isblank(Str_First[i_First - 1]) && isalpha(Str_Second[i_Second - 1]))
        return -((unsigned char)(tolower(Str_Second[i_Second - 1])));

      else if (isalpha(Str_First[i_First - 1]) && isblank(Str_Second[i_Second - 1]))
        return (unsigned char)(tolower(Str_First[i_First - 1]));

      else if ((i_First == 1) && (i_Second == 1))
        break;

      else
        continue;
    }

    else
      return (unsigned char)Letter1 - (unsigned char)Letter2;
  }

  return 0;
}

void Skip_Non_Letters(const char Str[], size_t *i)
{
  assert(Str);
  assert(i);
  
  while (Str[*i] != '\0' && !isalpha(Str[*i]))
    (*i)++;

  return;
}