/**
 * This example looks up a node from a document with an
 * xpath expression, then reports all properties that apply
 * from a given stylesheet.
 *
 * To compile it using gcc, type
 *
 * gcc `croco-config --cflags`  `croco-config --libs` -o selection-example-1 selection-example-1.c
 *
 * @author Stefan Seefeld <seefeld@sympatico.ca>
 */

#include <libcroco.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

void usage_and_exit(char *progname)
{
  fprintf(stderr, "Usage: %s <xml doc> <stylesheet> <xpath>\n", progname);
  exit(-1);
}

struct workspace
{
  xmlDoc *document;
  xmlXPathContext *xpath; 
  xmlXPathObject *result; 
  CRStyleSheet *stylesheet;
  CRCascade *cascade;
  CRSelEng *selector;
};

/** 
 *construct workspace members in order...
 * return 0 on success and -1 on error
 */
int init(struct workspace *ws, char **args)
{
  short i = 0;
  enum CRStatus status = CR_OK;

  ws->document = 0;
  ws->xpath = 0;
  ws->result = 0;
  ws->stylesheet = 0;
  ws->cascade = 0;
  ws->selector = 0;


  ws->document = xmlParseFile(args[0]);
  if (!ws->document)
  {
    fprintf(stderr, "could not parse the document %s", args[0]);
    return -1;
  }
  ws->xpath = xmlXPathNewContext(ws->document);
  if (!ws->xpath)
  {
    fprintf(stderr, "Error: unable to create new XPath context\n");
    return -1;
  }
  ws->result = xmlXPathEvalExpression((xmlChar *)args[2], ws->xpath);
  if (!ws->result)
  {
    fprintf(stderr, "Error: unable to evaluate xpath expression\n");
    return -1;
  }
  if (ws->result->type != XPATH_NODESET || !ws->result->nodesetval)
  {
    fprintf(stderr, "Error: xpath does not evaluate to a node set\n");
    return -1;
  }

  status = cr_om_parser_simply_parse_file((const guchar*)args[1] /*sheet*/,
                                          CR_ASCII /*the encoding*/,
                                          &ws->stylesheet);
  if (status != CR_OK || !ws->stylesheet)
  {
    fprintf(stderr, "could not parse the stylesheet %s", args[1]);
    return -1;
  }
  ws->cascade = cr_cascade_new(ws->stylesheet, 0, 0);
  ws->selector = cr_sel_eng_new();
}

/* ...and destruct in reverse order*/
void fini(struct workspace *ws)
{
  if (ws->selector) cr_sel_eng_destroy(ws->selector);
  if (ws->cascade) cr_cascade_destroy(ws->cascade);
  if (ws->stylesheet) cr_stylesheet_destroy(ws->stylesheet);
  if (ws->result) xmlXPathFreeObject(ws->result);
  if (ws->xpath) xmlXPathFreeContext(ws->xpath); 
  if (ws->document) xmlFreeDoc(ws->document);
}

void print_property(gpointer name, gpointer decl, gpointer data)
{
  CRDeclaration *declaration = (CRDeclaration *)decl;
  printf("%s\n", (char *)cr_declaration_to_string(declaration, 0));
}

void print_properties(struct workspace *ws)
{
  enum CRStatus status;
  GHashTable *table = g_hash_table_new(g_str_hash, g_str_equal);
  xmlNode *node = ws->result->nodesetval->nodeTab[0];
  if (!table)
  {
    fprintf(stderr, "unable to allocate a hash table\n");
    return;
  }
  status = cr_sel_eng_get_matched_properties_from_cascade(ws->selector,
                                                          ws->cascade,
                                                          node,
                                                          &table);
  if (status != CR_OK)
    fprintf(stderr, "Error retrieving properties\n");
  else
  {
    printf("properties for node %s :\n", (char *)xmlGetNodePath(node));
    g_hash_table_foreach(table, print_property, 0);
  }
  g_hash_table_destroy(table);
}

int main(int argc, char **argv)
{
  struct workspace ws;
  if (argc != 4) usage_and_exit(argv[0]);
  if (!init(&ws, argv + 1)) fini(&ws);

  if (ws.result->nodesetval->nodeNr == 0)
    printf("no matching nodes found\n");
  else
    print_properties(&ws);

  fini(&ws);
  return 0;
}
