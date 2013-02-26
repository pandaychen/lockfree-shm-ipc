#include "xml_wrapper.h"

mxml_node_t* xml_load_file(const char* file_name)
{
	if(!file_name)
		return NULL;
	
	FILE *fp;
	mxml_node_t* tree;
	fp = fopen(file_name, "r");
	if(!fp)
		return NULL;
	
	tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
	fclose(fp);
	return tree;
}

mxml_node_t* xml_find_child_element(mxml_node_t *parent,
		mxml_node_t *tree,
		const char* name)
{
	return mxmlFindElement(parent, tree, name, NULL, NULL, MXML_DESCEND_FIRST);
}

mxml_node_t* xml_find_sibling_element(mxml_node_t *node,
		mxml_node_t *tree,
		const char* name)
{
	return mxmlFindElement(node, tree, name, NULL, NULL, MXML_NO_DESCEND);
}

mxml_node_t* xml_get_next_sibling(mxml_node_t* node)
{
	return mxmlGetNextSibling(node);
}

const char* xml_element_get_text(mxml_node_t* node)
{
	if(!node)
		return NULL;

	return mxmlGetText(node, NULL);
}

void xml_release(mxml_node_t* tree)
{
	if(tree)
		mxmlRelease(tree);
}

