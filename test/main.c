#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <libxml2json2xml.h>

#include <NGAP-PDU.h>

int main(int argc, char **argv)
{

//	phase1) Read some bin file & convert to NGAP-PDU ASN

	FILE *fp = NULL;
	size_t file_size = 0;

	/* open aper pdu file */
	if (argc != 2) {
		fprintf(stderr, "Usage %s <pdu.filename>\n", argv[0]);
		exit(0);
	} else {
		if ((fp = fopen(argv[1], "rb")) == NULL) {
			fprintf(stderr, "Fail to read filename=[%s]!\n", argv[1]);
			exit(0);
		}
		/* read to pdu_payload_buffer */
		fseek(fp, 0, SEEK_END); file_size = ftell(fp); rewind(fp);
		fprintf(stderr, "Read processing [name:%s file_size:%ld]\n", argv[1], file_size);
	}
	/* create pdu buffer */
	char *pdu_payload_buffer = malloc(file_size); fread(pdu_payload_buffer, 1, file_size,  fp); fclose(fp);

	/* read NGAP-PDU payload asn */
    NGAP_PDU_t *pdu_payload_asn = NULL;
    asn_dec_rval_t dc_res = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_NGAP_PDU, (void **)&pdu_payload_asn, pdu_payload_buffer, file_size);
	fprintf(stderr, "Read %s, handle=(%ld) bytes rcode=(%d)\n", dc_res.code == RC_OK ? "OK" : "NOK", dc_res.consumed, dc_res.code);
    if (dc_res.code != RC_OK) {
        exit(0);
	}
	/* we don't need any more*/
	free(pdu_payload_buffer);

	/* check Constraint */
    char errbuf[128] = {0,}; size_t errlen = sizeof(errbuf);
    int ret = asn_check_constraints(&asn_DEF_NGAP_PDU, pdu_payload_asn, errbuf, &errlen);
	fprintf(stderr, "Check Constraint for %s, Result: %s Err=(%s)\n", asn_DEF_NGAP_PDU.name, ret ? "NOK" : "OK", errbuf);

	/* sample print */
	asn_fprint(stdout, &asn_DEF_NGAP_PDU, pdu_payload_asn);
	xer_fprint(stdout, &asn_DEF_NGAP_PDU, pdu_payload_asn);

// phase2) convert to XML string

	/* encode from ASN to XML : canonical mean no white space */
	asn_encode_to_new_buffer_result_t ec_xml = asn_encode_to_new_buffer(0, ATS_CANONICAL_XER, &asn_DEF_NGAP_PDU, pdu_payload_asn);
	fprintf(stderr, "Encode to XML Result: %s, Encoded=(%ld)\n", ec_xml.buffer == NULL ? "NOK" : "OK", ec_xml.result.encoded);
	if (ec_xml.buffer == NULL) {
		exit(0);
	}
	/* we dont need pdu_payload_asn anymore */
    ASN_STRUCT_FREE(asn_DEF_NGAP_PDU, pdu_payload_asn);

// phase3) convert to JSON string 

	/* encode from XML to JSON */
	json_object *jobj = json_object_new_object();
	xmlDoc *xml_doc = xmlReadMemory(ec_xml.buffer, ec_xml.result.encoded, NULL, NULL, 0);
	free(ec_xml.buffer);
	xmlNode *xml_node = xmlDocGetRootElement(xml_doc);
	xml2json_convert_elements(xml_node, jobj);
	fprintf(stderr, "%s\n", JSON_PRINT(jobj));
	/* we don't need any more */
	xmlFreeDoc(xml_doc);

	/* find NGAP-PDU & create to XML */
	json_object *jobj_ngap_pdu = json_object_object_get(jobj, "NGAP-PDU");
	xmlNode *new_xml = xmlNewNode(NULL, (xmlChar *)"NGAP-PDU");
	json2xml_convert_object(jobj_ngap_pdu, new_xml, json_type_object, NULL);
	xmlBuffer *new_buffer = xmlBufferCreate();
	int new_size = xmlNodeDump(new_buffer, NULL, new_xml, 0, 0);
	fprintf(stderr, "%s\n", new_buffer->content);
	/* we dont need any more */
	json_object_put(jobj);

    NGAP_PDU_t *new_pdu_payload_asn = NULL;
    dc_res = asn_decode(0, ATS_CANONICAL_XER, &asn_DEF_NGAP_PDU, (void **)&new_pdu_payload_asn, new_buffer->content, new_size); /* PDU == alsigned packet encoded file */
	fprintf(stderr, "Read %s, handle=(%ld) bytes rcode=(%d)\n", dc_res.code == RC_OK ? "OK" : "NOK", dc_res.consumed, dc_res.code);
    if (dc_res.code != RC_OK) {
        exit(0);
	}
	/* we dont need any more */
	xmlFreeNode(new_xml);
	xmlBufferFree(new_buffer);

	/* sample print */
	asn_fprint(stdout, &asn_DEF_NGAP_PDU, new_pdu_payload_asn);
	xer_fprint(stdout, &asn_DEF_NGAP_PDU, new_pdu_payload_asn);

// phase4 convert to APER PDU
    ret = asn_check_constraints(&asn_DEF_NGAP_PDU, new_pdu_payload_asn, errbuf, &errlen);
	fprintf(stderr, "Check Constraint for %s, Result: %s Err=(%s)\n", asn_DEF_NGAP_PDU.name, ret ? "NOK" : "OK", errbuf);

	/* encode from XML to APER */
	asn_encode_to_new_buffer_result_t ec_aper = asn_encode_to_new_buffer(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_NGAP_PDU, new_pdu_payload_asn);
	fprintf(stderr, "Encode to APER Result: %s, Encoded=(%ld)\n", ec_aper.buffer == NULL ? "NOK" : "OK", ec_aper.result.encoded);
	if (ec_aper.buffer == NULL) {
		exit(0);
	}
	/* we dont need pdu_payload_asn anymore */
    ASN_STRUCT_FREE(asn_DEF_NGAP_PDU, new_pdu_payload_asn);

    FILE *new_fp = fopen("./result.bin", "wb");
	fwrite(ec_aper.buffer, ec_aper.result.encoded, 1, new_fp);
	free(ec_aper.buffer);
	fclose(new_fp);

	xmlCleanupParser();
    return 1;
}
