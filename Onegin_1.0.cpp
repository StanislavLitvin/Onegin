#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#define NAME_OF_TESTING_FILE "Test_Onegina.txt"
#define NAME_OF_FILE_FROM "Eugene_Onegin.txt"
#define NAME_OF_FILE_TO "Sorted_Eugene_Onegin.txt"

size_t Read_From_File(const char *Name_File, const char *Index[]);
void Print_Strings_For_Debug(const char * const Index[], const size_t N_Lines, const char *Reason_Of_Using);
int Print_To_File(const char *Name_File, const char * const Index[], const size_t N_Lines, const char *Mode);
void Cleaning(const char * const Index[], const size_t N_Lines);

char *My_Strdup(const char Buffer[]);
size_t My_Strlen(const char Str[]);
char *My_Strcpy(char Str_To[], const char Str_From[]);

void Bubble_Sort(void *Array, size_t Length, size_t Size_Elem,
                 int (*Compare_Function)(const void *Number1, const void *Number2));
void Switch_Places(void *Value1, void *Value2, size_t Size_Elem);

int Compare_Str_Up(const void *Object1, const void *Object2);
int My_String_Strcmp(const char Str_First[], const char Str_Second[]);
int Compare_Str_Up_Reverse(const void *Object1, const void *Object2);
int My_String_Strcmp_Reverse(const char Str_First[], const char Str_Second[],
                             size_t Length_First, size_t Length_Second);

const size_t MAX_LENGTH_OF_LINE = 50;//in plans -- using realloc
const size_t NUMBER_OF_LINES = 5700;

int main()
{
  const char *Index[NUMBER_OF_LINES] = {0};

  const size_t N_Lines = Read_From_File(NAME_OF_FILE_FROM, Index);

  const char *Copy_Index[NUMBER_OF_LINES] = {0};
  for (size_t i = 0; i < N_Lines; i++)
    Copy_Index[i] = Index[i];

  Bubble_Sort(Index, N_Lines, sizeof(char *), &Compare_Str_Up);

  //Print_Strings_For_Debug(Index, N_Lines, "Testing bubble sort");

  Print_To_File(NAME_OF_FILE_TO, Index, N_Lines, "w");

  //Bubble_Sort(Index, N_Lines, sizeof(char *), &Compare_Str_Up_Reverse);
  qsort(Index, N_Lines, sizeof(char *), &Compare_Str_Up_Reverse);

  //Print_Strings_For_Debug(Index, N_Lines, "Testing reverse bubble sort");
  Print_To_File(NAME_OF_FILE_TO, Index, N_Lines, "a");

  Print_To_File(NAME_OF_FILE_TO, Copy_Index, N_Lines, "a");

  Cleaning(Index, N_Lines);
  Cleaning(Copy_Index, N_Lines);

  return 0;
}

size_t Read_From_File(const char *Name_File, const char *Index[])
{
  FILE *File = fopen(Name_File, "r");
  if (File == NULL)
  {
    perror(Name_File);
    return 0;
  }

  char Buffer[MAX_LENGTH_OF_LINE] = "";
  size_t N_Lines = 0;

  while (N_Lines < NUMBER_OF_LINES && fgets(Buffer, MAX_LENGTH_OF_LINE, File) != NULL)
  {
    Index[N_Lines] = My_Strdup(Buffer);
    
    if (Index[N_Lines] == NULL)
      break;

    N_Lines++;
  }

  fclose(File);

  return N_Lines;
}

int Print_To_File(const char *Name_File, const char * const Index[], const size_t N_Lines, const char *Mode)
{
  FILE * File = fopen(Name_File, Mode);
  if (File == NULL)
  {
    perror(Name_File);//TODO посмотреть как работает, показать на сдаче, использовать возвращаемые значения (функций, fopen, fclose...)
    return 0;//TODO assertы, отступы, сделать зануление ядами перед free, обертка для измерения размера файла, функция для выбрасывания всех вхождений, функция для счета количества вхождений, закинуть компараторы в структуру.
  }

  for (size_t i = 0; i < N_Lines; i++)
  {
    if ((Index[i])[0] == '\n')
      continue;

    else if (fputs(Index[i], File) == EOF)
      return -1;
  }

  fprintf(File, "\n\n\n----------------------------------END----------------------------------\n\n\n");

  fclose(File);

  return 1;
}

void Print_Strings_For_Debug(const char * const Index[], const size_t N_Lines, const char *Reason_Of_Using)
{
  for (size_t i = 0; i < N_Lines; i++)
  {
    printf("\x1b[32m<<%s>>\n\x1b[0m", Index[i]);
    printf("\t\tNumber of line: %zu\tAddress of line: %p\t"
           "Length of line: %zu\tReason of using: %s\n",
           i, Index[i], My_Strlen(Index[i]), Reason_Of_Using);
  }

  return;
}

void Cleaning(const char * const Index[], const size_t N_Lines)
{
  for (size_t i = 0; i < N_Lines; i++)
    free((void *) Index[i]);

  return;
}

char *My_Strdup(const char Buffer[])
{
  size_t Len = My_Strlen(Buffer);
  char *Pointer = (char *)calloc(Len + 1, sizeof(Buffer[0]));

  if (Pointer == NULL)
    return NULL;

  return My_Strcpy(Pointer, Buffer);
}

size_t My_Strlen(const char Str[])
{
  assert(Str);

  size_t Count = 0;
  while (Str[Count] != '\0')
    Count++;

  return Count;
}

char *My_Strcpy(char Str_To[], const char Str_From[])
{
  assert(Str_To);
  assert(Str_From);

  size_t Count = 0;

  while (Str_From[Count] != '\0')
  {
    Str_To[Count] = Str_From[Count];
    Count++;
  }

  Str_To[Count] = '\0';

  return Str_To;
}

void Bubble_Sort(void *Array, size_t Length, size_t Size_Elem,
                 int (*Compare_Function)(const void *Number1, const void *Number2))
{
  assert(Array);
  assert(Length > 0);
  assert(Size_Elem > 0);

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

  const char *String1 = *(const char * const *)Object1;
  const char *String2 = *(const char * const *)Object2;
  
  return My_String_Strcmp(String1, String2);
}

int My_String_Strcmp(const char Str_First[], const char Str_Second[])
{
  assert(Str_First);
  assert(Str_Second);

  size_t i_First = 0, i_Second = 0;

  if (Str_First[0] == '\n' && Str_Second[0] != '\n')
    return (unsigned char)(Str_First[0]);
  else if(Str_First[0] != '\n' && Str_Second[0] == '\n')
    return -((unsigned char)(Str_Second[0]));

  for (; Str_First[i_First + 1] != '\0' && Str_Second[i_Second + 1] != '\0'; i_First++, i_Second++)
  {
    while (Str_First[i_First] != '\0' && !isalpha(Str_First[i_First]))
      i_First++;

    while (Str_Second[i_Second] != '\0' && !isalpha(Str_Second[i_Second]))
      i_Second++;

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

  const char *String1 = *(const char * const *)Object1;
  const char *String2 = *(const char * const *)Object2;
  
  return My_String_Strcmp_Reverse(String1, String2, My_Strlen(String1), My_Strlen(String2));
}

int My_String_Strcmp_Reverse(const char Str_First[], const char Str_Second[],
                             size_t Length_First, size_t Length_Second)
{
  assert(Str_First);
  assert(Str_Second);
  assert(Length_First > 0);
  assert(Length_Second > 0);

  size_t i_First = Length_First - 1, i_Second = Length_Second - 1;

  if (Str_First[0] == '\n' && Str_Second[0] != '\n')
    return (unsigned char)(Str_First[0]);
  else if(Str_First[0] != '\n' && Str_Second[0] == '\n')
    return -((unsigned char)(Str_Second[0]));

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