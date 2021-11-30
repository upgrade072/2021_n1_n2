#include "libxml2json2xml.h"

json_object *attach_json_obj(xmlNode *cur_node, const char *cur_node_name, json_object *jobj, int is_leaf)
{
	xmlChar *content = is_leaf ? xmlNodeGetContent(cur_node) : NULL;
	json_object *cur_jobj = is_leaf ? json_object_new_string((const char *)content) : json_object_new_object();

	/* before add find sibling exist */
	json_object *already_exist = json_object_object_get(jobj, cur_node_name);
	if (!already_exist) {
		json_object_object_add(jobj, cur_node_name, cur_jobj);
	} else {
		/* if exist change to array type */
		if (json_object_get_type(already_exist) != json_type_array) {
			json_object *already_obj = is_leaf ? json_object_new_string(json_object_get_string(already_exist)) : json_object_get(already_exist);
			json_object_object_del(jobj, cur_node_name);

			json_object *new_array = json_object_new_array();
			json_object_object_add(jobj, cur_node_name, new_array);

			json_object_array_add(new_array, already_obj);
			json_object_array_add(new_array, cur_jobj);
		} else {
			json_object_array_add(already_exist, cur_jobj);
		}
	}

	if (content != NULL) { 
		xmlFree(content);
	}
	return cur_jobj;
}

void xml2json_convert_elements(xmlNode *anode, json_object *jobj)
{
    xmlNode *cur_node = NULL;
    json_object *cur_jobj = NULL;

    for (cur_node = anode; cur_node; cur_node = cur_node->next) {
		const char *cur_node_name = (const char *)cur_node->name;

        if (cur_node->type == XML_ELEMENT_NODE) {
			cur_jobj = attach_json_obj(cur_node, cur_node_name, jobj, !xmlChildElementCount(cur_node));
        }
        xml2json_convert_elements(cur_node->children, cur_jobj);
    }
}

int xml2json_sample(int argc, char **argv)
{
	xmlDoc *xml_doc = xmlReadFile(/* file name */argv[1], NULL, 0);
	xmlNode *xml = xmlDocGetRootElement(xml_doc);
	json_object *jobj = json_object_new_object();

	xml2json_convert_elements(xml, jobj);
	fprintf(stderr, "%s\n", JSON_PRINT(jobj));

	json_object_put(jobj);
	xmlFreeDoc(xml_doc);
	xmlCleanupParser();

	return 0;
}
