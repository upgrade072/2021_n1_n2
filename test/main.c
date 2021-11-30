#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <libxml2json2xml.h>

#include <NGAP-PDU.h>

#undef ASN_DEBUG
#define ASN_DEBUG(fmt, args...) do {            \
		fprintf(stderr,"????\n");	\
        int adi = asn_debug_indent;     \
        while(adi--) fprintf(stderr, " ");  \
        fprintf(stderr, fmt, ##args);       \
        fprintf(stderr, " (%s:%d)\n",       \
            __FILE__, __LINE__);        \
    } while(0)

int main(int argc, char **argv)
{
	/* open pdu file */
	if (argc != 2) {
		fprintf(stderr, "usage %s <pdu.filename>\n", argv[0]);
		exit(0);
	}
    const char *filename = argv[1];
    FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "fail to read filename=[%s]!\n", filename);
		exit(0);
	}

	/* read to pdu_payload */
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	rewind(fp);
	char *pdu_payload = malloc(size);
	fread(pdu_payload, 1, size,  fp);
	fclose(fp);

	/* read NAS PDU */
    NGAP_PDU_t *pdu_packet = NULL;
#if 1
    asn_dec_rval_t dc = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_NGAP_PDU, (void **)&pdu_packet, pdu_payload, size); /* PDU == alsigned packet encoded file */
#else
    asn_dec_rval_t dc = aper_decode(0, &asn_DEF_NGAP_PDU, (void **)&pdu_packet, pdu_payload, size, 0, 0); /* PDU == alsigned packet encoded file */
#endif
    if (dc.code != RC_OK) {
        fprintf(stderr, "=== %s: Broken encoding at byte %ld rc=(%d)\n", filename, dc.consumed, dc.code);
        exit(0);
    } else {
        fprintf(stderr, "=== Read success, handle=(%ld) bytes rc=(%d)\n", dc.consumed, dc.code);
    }

	/* check constraint */
    char errbuf[128] = {0,};
    size_t errlen = sizeof(errbuf);
    int ret = asn_check_constraints(&asn_DEF_NGAP_PDU, pdu_packet, errbuf, &errlen);
    if (ret) {
        fprintf(stderr, "=== Validation fail=[%s]\n", errbuf);
    } else {
        fprintf(stderr, "=== Validation ok!\n");
	}

	/* sample print */
	asn_fprint(stdout, &asn_DEF_NGAP_PDU, pdu_packet);
	xer_fprint(stdout, &asn_DEF_NGAP_PDU, pdu_packet);

	/* encode to XML : canonical mean no white space */
	char buffer[10240] = {0,}; // TODO dynamic size allocation !!!
	asn_enc_rval_t ec = asn_encode_to_buffer(0, ATS_CANONICAL_XER, &asn_DEF_NGAP_PDU, pdu_packet, buffer, sizeof(buffer));
	fprintf(stderr, "%s\n", buffer);
	fprintf(stderr, "=== encoding (%ld)bytes\n", ec.encoded);

	/* we dont need pdu_packet anymore */
    ASN_STRUCT_FREE(asn_DEF_NGAP_PDU, pdu_packet);

	/* encode --> json converting */
	xmlDoc *xml_doc = xmlReadMemory(buffer, ec.encoded, NULL, NULL, 0);
	xmlNode *xml = xmlDocGetRootElement(xml_doc);
	json_object *jobj = json_object_new_object();
	xml2json_convert_elements(xml, jobj);
	fprintf(stderr, "%s\n", JSON_PRINT(jobj));

	/* find NGAP-PDU & create to XML */
	json_object *jobj_ngap_pdu = json_object_object_get(jobj, "NGAP-PDU");
	xmlNode *new_xml = xmlNewNode(NULL, (xmlChar *)"NGAP-PDU");
	json2xml_convert_object(jobj_ngap_pdu, new_xml, json_type_object, NULL);
	xmlBuffer *new_buffer = xmlBufferCreate();
	int new_size = xmlNodeDump(new_buffer, NULL, new_xml, 0, 0);
	fprintf(stderr, "%s\n", new_buffer->content);

    NGAP_PDU_t *new_pdu_packet = NULL;
    dc = asn_decode(0, ATS_CANONICAL_XER, &asn_DEF_NGAP_PDU, (void **)&new_pdu_packet, new_buffer->content, new_size); /* PDU == alsigned packet encoded file */
    if (dc.code != RC_OK) {
        fprintf(stderr, "=== %s: Broken encoding at byte %ld\n", filename, (long)dc.consumed);
        exit(0);
    } else {
        fprintf(stderr, "=== Read success, handle=(%ld) bytes\n", dc.consumed);
    }

	/* sample print */
	asn_fprint(stdout, &asn_DEF_NGAP_PDU, new_pdu_packet);
	xer_fprint(stdout, &asn_DEF_NGAP_PDU, new_pdu_packet);

	char file_buffer[10240] = {0,}; // TODO dynamic size allocation !!!
	ec = asn_encode_to_buffer(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_NGAP_PDU, new_pdu_packet, file_buffer, sizeof(file_buffer));
	fprintf(stderr, "=== encoding (%ld)bytes\n", ec.encoded);

    FILE *new_fp = fopen("./result.bin", "wb");
	fwrite(file_buffer, ec.encoded, 1, new_fp);
	fclose(new_fp);

	/* TODO later */
    ASN_STRUCT_FREE(asn_DEF_NGAP_PDU, new_pdu_packet);
	xmlFreeNode(new_xml);
	xmlBufferFree(new_buffer);
	json_object_put(jobj);
	free(pdu_payload);
	xmlFreeDoc(xml_doc);
	xmlCleanupParser();
    return 1;
}
