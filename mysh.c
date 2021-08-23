#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define INPUT_LENGTH 200

/* Add arguments to the argument list. */
int add_argument(char*** list, int* number_of_arguments, char* string)
{
  /* If there are no arguments allocate memory for the argument list.
     If there already are arguments reallocate the memory for a new argument*/
  if ((*number_of_arguments) == 0 || (*list) == NULL)
    {
      (*number_of_arguments) = 1;
      (*list) = (char**)malloc(sizeof(char*));
	  if((*list) == NULL)
		{
		  char error_message[30] = "Failed to allocate memory\n";
		  write(STDERR_FILENO, error_message, strlen(error_message));
		}
    }
  else
    {
      (*number_of_arguments)++;
      (*list) = (char**)realloc((*list), (*number_of_arguments)*sizeof(char*));
    }
  
  /* Allocate memory for the argument. */
  (*list)[(*number_of_arguments)-1] = malloc(sizeof(char*));
  if((*list) == NULL)
	{
	  char error_message[30] = "Failed to allocate memory\n";
	  write(STDERR_FILENO, error_message, strlen(error_message));
	}
  
  /* Copy the argument into the allocated memory. */
  strcpy((*list)[(*number_of_arguments)-1], string);
  return 0;
}

/* Free memory allocated for the arguments. */
int free_arguments(char** list, int* number_of_arguments)
{
  /* Go through all the arguments. */
  while ((*number_of_arguments) > 0)
    {
	  /* Check if the argument is null. */
      if (list[(*number_of_arguments)-1] != NULL)
		{
		  /* Free memory allocated to the argument. */
		  free(list[(*number_of_arguments)-1]);
		}
      (*number_of_arguments)--;
    }
  return 0;
}

/* Process the input string into a list of arguments. */
int strmod(char* input, char*** arguments, int* number_of_arguments)
{
  char* input_string = (char*)malloc(INPUT_LENGTH * sizeof(char));
  if(input_string == NULL)
	{
	  char error_message[30] = "Failed to allocate memory\n";
	  write(STDERR_FILENO, error_message, strlen(error_message));
	}
  char* argument;
  strncpy(input_string, input, strlen(input)-1);
  /* Split the input string and add the first argument. */
  argument = strtok(input_string, " ");
  add_argument(arguments, number_of_arguments, argument);

  /* Go through the rest of the input string and add the arguments. */
  while (argument != NULL)
	{
	  argument = strtok(NULL, " ");
	  if (argument != NULL)
		add_argument(arguments, number_of_arguments, argument);
	}
  return 0;
}

int main(int argc, char** argv)
{
  size_t input_length = 200;
  char** arguments = NULL;
  int argument_number = 0;
  pid_t child_pid;
  int status = 0;
  char* input = NULL;
  /* Get user command and arguments. */
  fprintf(stdout, "> ");
  getline(&input, &input_length, stdin);

  /* Split user command and arguments into a list. */
  strmod(input, &arguments, &argument_number);

  /* Main loop for the shell */
  while(input)
    {
	  if (strcmp(arguments[0], "exit") == 0)
		{
		  if(argument_number > 1)
			{
			  char error_message[30] = "Exit has no arguments.\n";
			  write(STDERR_FILENO, error_message, strlen(error_message));
			  exit(0);
			}
		  else
			{
			  free(input);
			  free_arguments(arguments, &argument_number);
			  free(arguments);
			  return 0; 
			}
		}
	  /* Fork the process for executing the command. */
	  child_pid = fork();

	  /* Execute the command in child process*/
	  if (child_pid == 0)
		{
		  int err = execv(arguments[0], arguments);
		  if (err == -1)
			{
			  char error_message[30] = "execv failed\n";
			  write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
	  /* Parent process */
	  else if (child_pid > 0)
		{
		  /* Wait for the child process to stop. */
		  do
			{
			  waitpid(child_pid, &status, WUNTRACED | WCONTINUED);				
			}
		  while(!WIFEXITED(status) && !WIFSIGNALED(status));

		  free_arguments(arguments, &argument_number);
		  free(arguments);

		  /* Get and handle a new command from user. */
		  fprintf(stdout, "> ");
		  getline(&input, &input_length, stdin);
		  strmod(input, &arguments, &argument_number);
		}
    }
  return 0;
}
