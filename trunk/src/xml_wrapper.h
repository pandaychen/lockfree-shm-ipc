#ifndef XML_WRAPPER_H_
#define XML_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <mxml.h>

mxml_node_t* xml_load_file(const char* file_name);

mxml_node_t* xml_find_child_element(mxml_node_t *parent,
		mxml_node_t *tree,
		const char* name);

mxml_node_t* xml_find_sibling_element(mxml_node_t *node,
		mxml_node_t *tree,
		const char* name);

mxml_node_t* xml_get_next_sibling(mxml_node_t* node);

const char* xml_element_get_text(mxml_node_t* node);

void xml_release(mxml_node_t* tree);

#ifdef __cplusplus
}
#endif

#endif // XML_WRAPPER_H_

