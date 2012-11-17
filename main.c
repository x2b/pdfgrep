#include <stdio.h>
#include <gtk/gtk.h>
#include <poppler.h>
#include <stdlib.h>

char ESC=27;

void bold_on()
{
  printf("%c[1m",ESC);
}

void bold_off()
{
  printf("%c[0m",ESC);
}

void find(const char *filename, GRegex *regex)
{
  GFile           *file;
  gchar           *uri;
  PopplerDocument *doc;
  PopplerPage     *page;
  GError          *error = NULL;
  gint             i, n;
  gint             a, b;
  GMatchInfo      *match_info;
  gchar           *text;
  
  file = g_file_new_for_commandline_arg (filename);
  uri = g_file_get_uri (file);
  g_object_unref (file);

  if(!(doc = poppler_document_new_from_file (uri, NULL, &error)))
  {
    fprintf(stderr, "Could not open file %s: %s\n",
            filename, error->message);
    g_error_free(error);
    goto cleanup;
  }

  n = poppler_document_get_n_pages(doc);

  for(i = 0; i < n; ++i)
  {
    page = poppler_document_get_page(doc, i);
    text = poppler_page_get_text(page);
    
    g_regex_match(regex, text, (GRegexMatchFlags) 0, &match_info);
    
    while(g_match_info_matches(match_info))
    {
      bold_on();
      printf("%s:%i ", filename, i + 1);
      bold_off();
      
      g_match_info_fetch_pos(match_info, 0, &a, &b);
      
      for(; a >= 1 && *(text + a - 1) != '\n'; --a);
      for(; *(text + b) && *(text + b) != '\n'; ++b);
      
      
      //print out the entire line
      for(; a < b; ++a)
        printf("%c", *(text + a));
      printf("\n");
      
      g_match_info_next (match_info, NULL);
    }
    g_match_info_free(match_info);
    
    g_object_unref(page);
    
    g_free(text);
    
  }
  
  g_object_unref(doc);
  
cleanup:
  g_free(uri);
}

typedef struct argument
{
  const char *filename;
  GRegex *regex;
  pthread_t worker;
} argument;

void *find_par(void *ptr)
{
  argument *arg = (argument*) ptr;
  find(arg->filename, arg->regex);
  return 0;
}

int main(int argc, char **argv)
{
  const char      *filename;
  GError          *error = NULL;
  GRegex          *regex;
  int              i;

  gtk_init(&argc, &argv);

  if(argc < 3)
  {
    fprintf(stderr, "Usage: %s expression filenames\n" , argv[0]);
    return 1;
  }

  if(!(regex = g_regex_new(argv[1],
                           (GRegexCompileFlags) 0,
                           (GRegexMatchFlags) 0,
                           &error)))
  {
    fprintf(stderr, "Could not create regular expression: %s\n",
            error->message);
    g_error_free(error);
    return 1;
  }
  
  argc -= 2;
  argv += 2;
  
  argument *args = (argument*) malloc(argc* sizeof(argument));
  
  for(i = 0; i < argc; ++i)
  {
    argument *cur = args + i;
    cur->filename = argv[i];
    cur->regex = regex;
    //find(argv[i], regex);
    pthread_create(&cur->worker, NULL, find_par, cur);
  }
  
  for(i = 0; i < argc; ++i)
  {
    pthread_join(args[i].worker, NULL);
  }
  
  g_regex_unref(regex);
  
  free(args);

  return 0;
}
