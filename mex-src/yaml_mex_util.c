#include "yaml_mex_util.h"

/* NOTES
 * 
 * The Matlab C API documentation is maddeningly vague about how Unicode is
 * supported exactly. It seems that internally, character arrays use
 * UTF-16, and (on my machine at least) are big-endian. According to the
 * documentation, mxArrayToString "supports multibyte character sets,"
 * though it outputs single-byte char*. Perhaps this means that it outputs
 * UTF-8 encoded Unicode? The documentation does not mention whether
 * mxCreateString similarly "supports multibyte character sets." I'm going
 * to work under the assumption that letting libyaml process UTF-8 and
 * feeding that to Matlab through those two functions is safe. If not,
 * then I'll have to squirrel around with UTF-16 encoding, and probably
 * need to strip off the BOM (since Matlab doesn't use it internally). I'll
 * also need to figure out some way to determine whether Matlab is always
 * big-endian, or if it varies by machine, and if so, how to tell.
 */

/*
 * Internal datatype declarations
 *************************************************************************/

typedef struct ymx_document_s {
    mxArray *root;
    mxArray *tagdirs;
    mxArray *version;
    mxArray *start_implicit;
    mxArray *end_implicit;
} ymx_document_t;

typedef struct ymx_node_s {
    mxArray *type;
    mxArray *value;
    mxArray *tag;
    mxArray *anchor;
    mxArray *implicit;
    mxArray *style;
} ymx_node_t;

typedef struct ymx_buffer_s {
    size_t chunk_size; /* Size of a chunk, in bytes */
    size_t total_size; /* Current total size of the buffer, in bytes */
    size_t used_size;  /* Amount of the buffer currently used, in bytes */
    void *head; /* Pointer to the head of the buffer */
} ymx_buffer_t;
typedef char ymx_buffer_char_t;


/*
 * Constant definitions
 *************************************************************************/
const char *YMX_NODE_TYPE_STRS[] =
        {"none", "scalar", "sequence", "mapping", "alias"};
const char *YMX_NODE_FIELD_STRS[] =
        {"type", "value", "tag", "anchor", "implicit", "style"};
const char *YMX_DOC_FIELD_STRS[] =
        {"root", "version", "tagdirs", "start_implicit", "end_implicit"};
const char *YMX_TAGDIR_FIELD_STRS[] =
        {"handle", "prefix"};

const size_t YMX_BUFFER_TAIL_SIZE = sizeof(ymx_buffer_char_t);
const size_t YMX_BUFFER_CHUNK_SIZE = 1024;
const size_t NODES_BUFFER_CHUNK_SIZE = 128;


/*
 * Internal function declarations
 *************************************************************************/

mxArray *ymx_create_int_scalar(int32_T value);
int32_T  ymx_get_int_scalar(mxArray *scalar);

void ymx_set_node_fields(mxArray *array, mwIndex ind, ymx_node_t *node);

const yaml_event_t *ymx_parse(yaml_parser_t *parser);
void ymx_emit(yaml_emitter_t *emitter, yaml_event_t *event);
void ymx_error(const char* message);

void ymx_load_document(
        ymx_document_t *doc,
        yaml_parser_t *parser,
        const yaml_event_t *first_event );
void ymx_load_node(
        ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event );
void ymx_load_scalar(
        ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event );
void ymx_load_alias(
        ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event );
void ymx_load_sequence(
        ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event );
void ymx_load_mapping(
        ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event );

void ymx_dump_document(
        yaml_emitter_t *emitter,
        const mxArray *docs_array,
        mwIndex ind );
void ymx_dump_node(
        yaml_emitter_t *emitter,
        const mxArray *node,
        mwIndex ind );
void ymx_dump_scalar(
        yaml_emitter_t *emitter,
        mxArray *value_arr,
        yaml_char_t *tag,
        yaml_char_t *anchor,
        int32_T implicit,
        int32_T style );
void ymx_dump_sequence(
        yaml_emitter_t *emitter,
        mxArray *value_arr,
        yaml_char_t *tag,
        yaml_char_t *anchor,
        int32_T implicit,
        int32_T style );
void ymx_dump_mapping(
        yaml_emitter_t *emitter,
        mxArray *value_arr,
        yaml_char_t *tag,
        yaml_char_t *anchor,
        int32_T implicit,
        int32_T style );
void ymx_dump_alias(
        yaml_emitter_t *emitter,
        yaml_char_t *anchor );

int ymx_is_valid_doc_array(const mxArray *doc_array);
int ymx_is_valid_version_array(const mxArray *version);
int ymx_is_valid_tagdirs_array(const mxArray *tagdirs);
int ymx_is_valid_doc_root(const mxArray *root);
int ymx_is_valid_node_array(const mxArray *node);
int ymx_is_valid_int_scalar(const mxArray *scalar);
int ymx_is_valid_string(const mxArray *string);

void ymx_buffer_init(ymx_buffer_t *buffer, size_t chunk_size);
void ymx_buffer_delete(ymx_buffer_t *buffer);
void ymx_buffer_append(ymx_buffer_t *buffer, 
        void *input, size_t size);
ymx_buffer_char_t *ymx_buffer_as_string(ymx_buffer_t *buffer);
static int ymx_buffer_write_handler(void *data,
        unsigned char *buffer, size_t size);


/*
 * Globals; needed so we can clean up memory leaks if there's a problem.
 *************************************************************************/
/* Supposedly these will all be initialized to zeros with static lifetime. */
static yaml_parser_t   persistent_parser;
static yaml_emitter_t  persistent_emitter;
static yaml_event_t    persistent_event;

void ymx_persistent_cleanup() {
    yaml_parser_delete(&persistent_parser);
    yaml_emitter_delete(&persistent_emitter);
    yaml_event_delete(&persistent_event);
}

/*
 * Utility functions
 *************************************************************************/

void ymx_error(const char *message) {
    ymx_persistent_cleanup();
    mexErrMsgTxt(message);
}

void ymx_set_node_fields(mxArray *array, mwIndex ind, ymx_node_t *node) {
    ymx_debug_msg("Entering ymx_set_node_fields\n");
    mxSetFieldByNumber(array, ind,
            YMX_NODE_FIELD_TYPE, node->type);
    mxSetFieldByNumber(array, ind,
            YMX_NODE_FIELD_VALUE, node->value);
    mxSetFieldByNumber(array, ind,
            YMX_NODE_FIELD_TAG, node->tag);
    mxSetFieldByNumber(array, ind,
            YMX_NODE_FIELD_ANCHOR, node->anchor);
    mxSetFieldByNumber(array, ind,
            YMX_NODE_FIELD_IMPLICIT, node->implicit);
    mxSetFieldByNumber(array, ind,
            YMX_NODE_FIELD_STYLE, node->style);
    ymx_debug_msg("Exiting ymx_set_node_fields\n");
}

const yaml_event_t *ymx_parse(yaml_parser_t *parser) {
    /* I really hate doing this, but I can't think of another good way to
     * keep from leaking memory if Matlab generates an error. */
    yaml_event_delete(&persistent_event);
    if (!yaml_parser_parse(parser, &persistent_event)) {
        mexPrintf("Parser error: %s\n", parser->problem);
        ymx_error("Error while parsing document");
    }
    return &persistent_event;
}

void ymx_emit(yaml_emitter_t *emitter, yaml_event_t *event) {
    if (!yaml_emitter_emit(emitter, event)) {
        mexPrintf("Emitter error: %s\n", emitter->problem);
        ymx_error("Error while emitting document");
    }
}

mxArray *ymx_create_int_scalar(int32_T value) {
    mxArray *output = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
    ((int32_T *)mxGetData(output))[0] = value;
    return output;
}

int32_T ymx_get_int_scalar(mxArray *scalar) {
    return ((int32_T *)mxGetData(scalar))[0];
}

/* Determine whether a document array is valid. This checks only the
 * document array itself, not its members.
 * Returns 1 iff doc_array is a struct, has YMX_DOC_NUM_FIELDS fields, and
 * the field names match YMX_DOC_FIELD_STRS; otherwise returns 0.
 */
int ymx_is_valid_doc_array(const mxArray *doc_array) {
    if (!mxIsStruct(doc_array))
        return 0;
    
    if (mxGetNumberOfFields(doc_array) != YMX_DOC_NUM_FIELDS)
        return 0;
    
    int i;
    for (i=0; i<YMX_DOC_NUM_FIELDS; i++) {
        if (strcmp( mxGetFieldNameByNumber(doc_array, i),
                YMX_DOC_FIELD_STRS[i] ) != 0)
            return 0;
    }
    
    return 1;
}

/* Determine whether a version array (for a document) is valid. Returns 1
 * iff version is an int32 with 2 elements; otherwise returns 0.
 */
int ymx_is_valid_version_array(const mxArray *version) {
    mwSize size = mxGetNumberOfElements(version);
    if (size != 2)
        return 0;
    
    if (!mxIsInt32(version))
        return 0;
    
    return 1;
}

/* Determine whether a tagdirs array (for a document) is valid. Returns 1
 * iff tagdirs is a struct, has YMX_TAGDIRS_NUM_FIELDS fields, the field
 * names match YMX_TAGDIRS_FIELD_STRS, and every struct member is a string.
 */
int ymx_is_valid_tagdirs_array(const mxArray *tagdirs) {
    if (!mxIsStruct(tagdirs))
        return 0;
    
    if (mxGetNumberOfFields(tagdirs) != YMX_TAGDIR_NUM_FIELDS)
        return 0;
    
    mwSize size = mxGetNumberOfElements(tagdirs);
    mwIndex field_ind;
    mwIndex array_ind;
    mxArray *member;
    for (field_ind=0; field_ind<YMX_TAGDIR_NUM_FIELDS; field_ind++) {
        if (strcmp( mxGetFieldNameByNumber(tagdirs, field_ind),
                YMX_TAGDIR_FIELD_STRS[field_ind] ) != 0)
            return 0;
        
        for (array_ind=0; array_ind<size; array_ind++) {
            member = mxGetFieldByNumber(tagdirs, array_ind, field_ind);
            if (mxGetNumberOfElements(member) == 0)
                return 0; /* Empty tag elements not okay */
            
            if (!ymx_is_valid_string(member))
                return 0;
        }
    }
    
    return 1;
}

int ymx_is_valid_doc_root(const mxArray *root) {
    if (!root)
        return 0;
    
    if (mxGetNumberOfElements(root) != 1)
        return 0;
    
    return ymx_is_valid_node_array(root);
}

/* Determine whether a node array is valid. This checks only the structure
 * of the node array, not the values of any of its members.
 * Returns 1 iff node is a struct, with YMX_NODE_NUM_FIELDS fields, and the
 * field names match YMX_NODE_FIELD_STRS.
 */
int ymx_is_valid_node_array(const mxArray *node) {
    if (!mxIsStruct(node))
        return 0;
    
    if (mxGetNumberOfFields(node) != YMX_NODE_NUM_FIELDS)
        return 0;
    
    mwIndex field_ind;
    for (field_ind = 0; field_ind < YMX_NODE_NUM_FIELDS; field_ind++) {
        if (strcmp( mxGetFieldNameByNumber(node, field_ind),
                YMX_NODE_FIELD_STRS[field_ind] ) != 0)
            return 0;
    }
    
    return 1;
}

int ymx_is_valid_int_scalar(const mxArray *scalar) {
    if (!mxIsInt32(scalar))
        return 0;
    
    if (mxGetNumberOfElements(scalar) != 1)
        return 0;
    
    return 1;
}

int ymx_is_valid_string(const mxArray *string) {
    if (!mxIsChar(string))
        return 0;
    
    if (mxGetNumberOfElements(string) == 0)
        return 1; /* Empty strings are okay */
    
    if (mxGetNumberOfDimensions(string) != 2)
        return 0;
    
    if (mxGetM(string) != 1)
        return 0;
    
    if (mxGetN(string) < 1)
        return 0;
    
    return 1;
}


/*
 * Functions for loading
 *************************************************************************/

mxArray *ymx_load_stream(const mxArray *yaml_stream) {
    ymx_debug_msg("Entering ymx_load_stream\n");
    ymx_persistent_cleanup();
    
    mwSize num_docs = 0;
    char *yaml_cstr = mxArrayToString(yaml_stream);
    ymx_document_t *raw_docs = NULL;
    
    yaml_parser_t *parser = &persistent_parser;
    yaml_parser_initialize(parser);
    yaml_parser_set_input_string(parser,
            (yaml_char_t *)yaml_cstr, strlen(yaml_cstr));
    
    const yaml_event_t *event = ymx_parse(parser);
    mxAssert(event->type == YAML_STREAM_START_EVENT,
            "Expected stream start event!");
    event = ymx_parse(parser);
    while (event->type != YAML_STREAM_END_EVENT) {
        if (raw_docs) {
            raw_docs = mxRealloc(raw_docs,
                    (num_docs+1)*sizeof(ymx_document_t));
            memset(raw_docs + num_docs, 0, sizeof(ymx_document_t));
        } else {
            raw_docs = mxCalloc(1, sizeof(ymx_document_t));
        }
        ymx_load_document(raw_docs+num_docs, parser, event);
        event = ymx_parse(parser);
        num_docs++;
    }
    
    mxArray *docs_array = NULL;
    if (num_docs) {
        docs_array = mxCreateStructMatrix(1, num_docs,
                YMX_DOC_NUM_FIELDS, YMX_DOC_FIELD_STRS);
        mwIndex i;
        for (i=0; i<num_docs; i++) {
            mxSetFieldByNumber(docs_array, i,
                    YMX_DOC_FIELD_ROOT, raw_docs[i].root);
            mxSetFieldByNumber(docs_array, i,
                    YMX_DOC_FIELD_VERSION, raw_docs[i].version);
            mxSetFieldByNumber(docs_array, i,
                    YMX_DOC_FIELD_TAGDIRS, raw_docs[i].tagdirs);
            mxSetFieldByNumber(docs_array, i,
                    YMX_DOC_FIELD_START_IMPLICIT, raw_docs[i].start_implicit);
            mxSetFieldByNumber(docs_array, i,
                    YMX_DOC_FIELD_END_IMPLICIT, raw_docs[i].end_implicit);
        }
    }
    
    mxFree(yaml_cstr);
    mxFree(raw_docs);
    ymx_debug_msg("Exiting ymx_load_stream\n");
    return docs_array;
}

void ymx_load_document( ymx_document_t *doc,
        yaml_parser_t *parser,
        const yaml_event_t *first_event ) {
    ymx_debug_msg("Entering ymx_load_document\n");
    mxAssert(first_event->type == YAML_DOCUMENT_START_EVENT,
            "Expected document start event!");
    yaml_version_directive_t *vers_dir
            = first_event->data.document_start.version_directive;
    if (vers_dir) {
        doc->version = mxCreateNumericMatrix(1, 2, mxINT32_CLASS, mxREAL);
        int32_T *version_data = mxGetData(doc->version);
        version_data[0] = vers_dir->major;
        version_data[1] = vers_dir->minor;
    }
    
    yaml_tag_directive_t *yaml_tagdirs =
            first_event->data.document_start.tag_directives.start;
    
    mwSize num_dirs = first_event->data.document_start.tag_directives.end
            - yaml_tagdirs;
    if (num_dirs) {
        doc->tagdirs = mxCreateStructMatrix(1, num_dirs,
                YMX_TAGDIR_NUM_FIELDS, YMX_TAGDIR_FIELD_STRS);
        mwIndex ind;
        for (ind=0; ind<num_dirs; ind++) {
            mxSetFieldByNumber(doc->tagdirs, ind,
                    YMX_TAGDIR_FIELD_HANDLE,
                    mxCreateString((char *)yaml_tagdirs[ind].handle));
            mxSetFieldByNumber(doc->tagdirs, ind,
                    YMX_TAGDIR_FIELD_PREFIX,
                    mxCreateString((char *)yaml_tagdirs[ind].prefix));
        }
    }
    
    doc->start_implicit = mxCreateLogicalScalar(
            first_event->data.document_start.implicit );
    
    ymx_node_t node;
    memset(&node, 0, sizeof(node));
    const yaml_event_t *event = ymx_parse(parser);
    ymx_load_node(&node, parser, event);
    
    event = ymx_parse(parser);
    mxAssert(event->type == YAML_DOCUMENT_END_EVENT,
            "Expected document end!");
    doc->end_implicit = mxCreateLogicalScalar(
            event->data.document_end.implicit );
    
    doc->root = mxCreateStructMatrix(1, 1,
            YMX_NODE_NUM_FIELDS, YMX_NODE_FIELD_STRS);
    ymx_set_node_fields(doc->root, 0, &node);
    ymx_debug_msg("Exiting ymx_load_document\n");
}

void ymx_load_node( ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event ) {
    ymx_debug_msg("Entering ymx_load_node\n");
    switch (first_event->type) {
        case YAML_ALIAS_EVENT:
            ymx_load_alias(node, parser, first_event);
            break;
        case YAML_SCALAR_EVENT:
            ymx_load_scalar(node, parser, first_event);
            break;
        case YAML_SEQUENCE_START_EVENT:
            ymx_load_sequence(node, parser, first_event);
            break;
        case YAML_MAPPING_START_EVENT:
            ymx_load_mapping(node, parser, first_event);
            break;
    }
    ymx_debug_msg("Exiting ymx_load_node\n");
}

void ymx_load_scalar( ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event ) {
    ymx_debug_msg("Entering ymx_load_scalar\n");
    yaml_char_t *tag = first_event->data.scalar.tag;
    if (tag) {
        node->tag = mxCreateString((char *)tag);
    } else if (first_event->data.scalar.style == YAML_PLAIN_SCALAR_STYLE) {
        node->tag = mxCreateString("?");
    } else {
        node->tag = mxCreateString("!");
    }
    
    if (first_event->data.scalar.plain_implicit) {
        node->implicit = ymx_create_int_scalar(YMX_SCALAR_IMPLICIT_PLAIN);
    } else if (first_event->data.scalar.quoted_implicit) {
        node->implicit = ymx_create_int_scalar(YMX_SCALAR_IMPLICIT_QUOTED);
    } else {
        node->implicit = ymx_create_int_scalar(YMX_SCALAR_IMPLICIT_FALSE);
    }
    
    yaml_char_t *anchor = first_event->data.scalar.anchor;
    if (anchor) {
        node->anchor = mxCreateString((char *)anchor);
    }
    
    node->style = ymx_create_int_scalar(first_event->data.scalar.style);
    node->type = ymx_create_int_scalar(YMX_NODE_TYPE_SCALAR);
    
    if (first_event->data.scalar.value) {
        mwSize length = first_event->data.scalar.length;
        char *value = mxCalloc(length + 1, sizeof(char));
        memcpy(value, first_event->data.scalar.value, length);
        
        node->value = mxCreateString(value);
        mxFree(value);
    }
    
    ymx_debug_msg("Exiting ymx_load_scalar\n");
}

void ymx_load_alias( ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event ) {
    ymx_debug_msg("Entering ymx_load_alias\n");
    node->anchor = mxCreateString((char *)first_event->data.alias.anchor);
    
    node->type = ymx_create_int_scalar(YMX_NODE_TYPE_ALIAS);
    node->style = ymx_create_int_scalar(0);
    node->implicit = ymx_create_int_scalar(0);
    
    ymx_debug_msg("Exiting ymx_load_alias\n");
}

void ymx_load_sequence( ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event ) {
    ymx_debug_msg("Entering ymx_load_sequence\n");
    yaml_char_t *tag = first_event->data.sequence_start.tag;
    if (tag) {
        node->tag = mxCreateString((char *)tag);
    } else {
        node->tag = mxCreateString("?");
    }
    
    yaml_char_t *anchor = first_event->data.sequence_start.anchor;
    if (anchor) {
        node->anchor = mxCreateString((char *)anchor);
    }
    
    node->implicit = ymx_create_int_scalar(
            first_event->data.sequence_start.implicit );
    node->style = ymx_create_int_scalar(
            first_event->data.sequence_start.style );
    node->type = ymx_create_int_scalar(YMX_NODE_TYPE_SEQUENCE);
    
    size_t items_buffer_size = NODES_BUFFER_CHUNK_SIZE;
    ymx_node_t *items = mxCalloc(items_buffer_size, sizeof(ymx_node_t));
    size_t num_items = 0;
    
    const yaml_event_t *event = ymx_parse(parser);
    
    while (event->type != YAML_SEQUENCE_END_EVENT) {
        if (num_items == items_buffer_size) {
            items_buffer_size += NODES_BUFFER_CHUNK_SIZE;
            items = mxRealloc(items,
                    items_buffer_size * sizeof(ymx_node_t));
            memset(items + num_items, 0,
                    NODES_BUFFER_CHUNK_SIZE * sizeof(ymx_node_t));
        }
        ymx_load_node(items + num_items, parser, event);
        num_items++;
        
        event = ymx_parse(parser);
    }
    
    if (num_items) {
        node->value = mxCreateStructMatrix(1, num_items,
                YMX_NODE_NUM_FIELDS, YMX_NODE_FIELD_STRS);
        int i;
        for (i=0; i<num_items; i++) {
            ymx_set_node_fields(node->value, i, items+i);
        }
    }
    
    mxFree(items);
    ymx_debug_msg("Exiting ymx_load_sequence\n");
}

void ymx_load_mapping( ymx_node_t *node,
        yaml_parser_t *parser,
        const yaml_event_t *first_event ) {
    ymx_debug_msg("Entering ymx_load_mapping\n");
    yaml_char_t *tag = first_event->data.mapping_start.tag;
    if (tag) {
        node->tag = mxCreateString((char *)tag);
    } else {
        node->tag = mxCreateString("?");
    }
    
    yaml_char_t *anchor = first_event->data.mapping_start.anchor;
    if (anchor) {
        node->anchor = mxCreateString((char *)anchor);
    }
    
    node->implicit = ymx_create_int_scalar(
            first_event->data.mapping_start.implicit );
    node->style = ymx_create_int_scalar(
            first_event->data.mapping_start.style );
    node->type = ymx_create_int_scalar(YMX_NODE_TYPE_MAPPING);
    
    size_t buffer_size = NODES_BUFFER_CHUNK_SIZE;
    ymx_node_t *keys   = mxCalloc(buffer_size, sizeof(ymx_node_t));
    ymx_node_t *values = mxCalloc(buffer_size, sizeof(ymx_node_t));
    size_t num_items = 0;
    
    const yaml_event_t *event = ymx_parse(parser);
    
    while (event->type != YAML_MAPPING_END_EVENT) {
        if (num_items == buffer_size) {
            buffer_size += NODES_BUFFER_CHUNK_SIZE;
            keys = mxRealloc(keys,
                    buffer_size * sizeof(ymx_node_t));
            memset(keys + num_items, 0,
                    NODES_BUFFER_CHUNK_SIZE * sizeof(ymx_node_t));
            values = mxRealloc(values,
                    buffer_size * sizeof(ymx_node_t));
            memset(values + num_items, 0,
                    NODES_BUFFER_CHUNK_SIZE * sizeof(ymx_node_t));
        }
        
        ymx_load_node(keys + num_items, parser, event);
        event = ymx_parse(parser);
        ymx_load_node(values + num_items, parser, event);
        num_items++;
        event = ymx_parse(parser);
    }
    
    if (num_items) {
        node->value = mxCreateStructMatrix(2, num_items,
                YMX_NODE_NUM_FIELDS, YMX_NODE_FIELD_STRS);
        int i;
        for (i=0; i<num_items; i++) {
            ymx_set_node_fields(node->value, 2*i, keys+i);
            ymx_set_node_fields(node->value, 2*i+1, values+i);
        }
    }
    
    mxFree(keys);
    mxFree(values);
    ymx_debug_msg("Exiting ymx_load_mapping\n");
}


/*
 * Functions for dumping
 *************************************************************************/

mxArray *ymx_dump_stream(const mxArray *docs_array) {
    ymx_persistent_cleanup();
    if (!ymx_is_valid_doc_array(docs_array))
        ymx_error("Invalid document array");
    
    ymx_buffer_t buffer;
    ymx_buffer_init(&buffer, YMX_BUFFER_CHUNK_SIZE);
    
    yaml_emitter_t *emitter = &persistent_emitter;
    yaml_emitter_initialize(emitter);
    yaml_emitter_set_output(emitter, ymx_buffer_write_handler, &buffer);
    
    yaml_event_t *event = &persistent_event;
    
    if (!yaml_stream_start_event_initialize(event, YAML_ANY_ENCODING))
        ymx_error("Couldn't initialize event");
    
    ymx_emit(emitter, event);
    
    mwSize num_docs = mxGetNumberOfElements(docs_array);
    mwIndex ind;
    for (ind=0; ind<num_docs; ind++) {
        ymx_dump_document(emitter, docs_array, ind);
    }
    
    yaml_stream_end_event_initialize(event);
    ymx_emit(emitter, event);
    
    yaml_emitter_delete(emitter);
    
    mxArray *stream = mxCreateString(ymx_buffer_as_string(&buffer));
    
    ymx_buffer_delete(&buffer);
    
    return stream;
}

void ymx_dump_document(yaml_emitter_t *emitter,
        const mxArray *docs_array, mwIndex ind) {
    
    ymx_document_t doc;
    doc.root = mxGetFieldByNumber( docs_array, ind,
            YMX_DOC_FIELD_ROOT );
    if (!doc.root || !ymx_is_valid_doc_root(doc.root))
        ymx_error("Invalid document root");
    
    /* Assemble tag directives */
    doc.tagdirs = mxGetFieldByNumber( docs_array, ind,
            YMX_DOC_FIELD_TAGDIRS );
    
    yaml_tag_directive_t *tag_directives_start;
    yaml_tag_directive_t *tag_directives_end;
    mwSize num_tag_directives;
    mwIndex tagdir_ind;
    
    int has_tagdirs = (doc.tagdirs && !mxIsEmpty(doc.tagdirs));
    if (has_tagdirs) {
        if (!ymx_is_valid_tagdirs_array(doc.tagdirs))
            ymx_error("Invalid document tag directives");
        
        num_tag_directives = mxGetNumberOfElements(doc.tagdirs);
        
        tag_directives_start = mxCalloc( num_tag_directives,
                sizeof(yaml_tag_directive_t) );
        tag_directives_end = tag_directives_start + num_tag_directives;
        
        for (tagdir_ind=0; tagdir_ind<num_tag_directives; tagdir_ind++) {
            tag_directives_start[tagdir_ind].handle =
                    (yaml_char_t *) mxArrayToString(
                        mxGetFieldByNumber(doc.tagdirs, tagdir_ind,
                            YMX_TAGDIR_FIELD_HANDLE) );
            tag_directives_start[tagdir_ind].prefix =
                    (yaml_char_t *) mxArrayToString(
                        mxGetFieldByNumber(doc.tagdirs, tagdir_ind,
                            YMX_TAGDIR_FIELD_PREFIX) );
        }
        
    } else {
        num_tag_directives   = 0;
        tag_directives_start = NULL;
        tag_directives_end   = NULL;
    }
    
    yaml_version_directive_t *version_directive = NULL;
    yaml_version_directive_t version_directive_data;
    int32_T *version_data;
    
    doc.version = mxGetFieldByNumber( docs_array, ind,
            YMX_DOC_FIELD_VERSION );
    int has_version = (doc.version && !mxIsEmpty(doc.version));
    if (has_version) {
        if (!ymx_is_valid_version_array(doc.version))
            ymx_error("Invalid document version");
        
        version_data = mxGetData(doc.version);
        version_directive_data.major = version_data[0];
        version_directive_data.minor = version_data[1];
        version_directive = &version_directive_data;
    }
    
    int32_T start_implicit;
    doc.start_implicit = mxGetFieldByNumber( docs_array, ind,
            YMX_DOC_FIELD_START_IMPLICIT );
    if ( doc.start_implicit
            && mxIsLogicalScalar(doc.start_implicit) ) {
        start_implicit = mxGetLogicals(doc.start_implicit)[0];
    } else {
        start_implicit = 0;
        mexWarnMsgTxt("Invalid document start implicit specifier");
    }
    
    int32_T end_implicit;
    doc.end_implicit = mxGetFieldByNumber( docs_array, ind,
            YMX_DOC_FIELD_END_IMPLICIT );
    if ( doc.end_implicit
            && mxIsLogicalScalar(doc.end_implicit) ) {
        end_implicit = mxGetLogicals(doc.end_implicit)[0];
    } else {
        end_implicit = 0;
        mexWarnMsgTxt("Invalid document end implicit specifier");
    }
    
    yaml_event_t *event = &persistent_event;
    
    yaml_document_start_event_initialize(event,
            version_directive,
            tag_directives_start,
            tag_directives_end,
            start_implicit);
    ymx_emit(emitter, event);
    
    ymx_dump_node(emitter, doc.root, 0);
    
    yaml_document_end_event_initialize(event, end_implicit);
    ymx_emit(emitter, event);
    
    /* Free memory allocated for tag directives */
    for (tagdir_ind=0; tagdir_ind < num_tag_directives; tagdir_ind++) {
        mxFree(tag_directives_start[tagdir_ind].handle);
        mxFree(tag_directives_start[tagdir_ind].prefix);
    }
    mxFree(tag_directives_start);
}

void ymx_dump_node(
        yaml_emitter_t *emitter,
        const mxArray *node,
        mwIndex ind ) {
    /* Assemble tag */
    mxArray *tag_arr = mxGetFieldByNumber( node, ind,
            YMX_NODE_FIELD_TAG );
    
    yaml_char_t *tag_str = NULL;
    if (tag_arr && !mxIsEmpty(tag_arr)) {
        if (!ymx_is_valid_string(tag_arr))
            ymx_error("Invalid tag");
        
        tag_str = (yaml_char_t *)mxArrayToString(tag_arr);
    }
    
    /* Assemble anchor */
    mxArray *anchor_arr = mxGetFieldByNumber( node, ind,
            YMX_NODE_FIELD_ANCHOR );
    yaml_char_t *anchor_str = NULL;
    if (anchor_arr && !mxIsEmpty(anchor_arr)) {
        if (!ymx_is_valid_string(anchor_arr))
            ymx_error("Invalid anchor");
        
        anchor_str = (yaml_char_t *)mxArrayToString(anchor_arr);
    }
    
    /* Assemble implicit */
    mxArray *implicit_arr = mxGetFieldByNumber( node, ind,
            YMX_NODE_FIELD_IMPLICIT );
    int32_T implicit_int = 0;
    if ( implicit_arr && ymx_is_valid_int_scalar(implicit_arr) ) {
        implicit_int = ymx_get_int_scalar(implicit_arr);
    } else {
        mexWarnMsgTxt("Invalid node implicit specifier");
    }
    
    /* Assemble style */
    mxArray *style_arr = mxGetFieldByNumber( node, ind,
            YMX_NODE_FIELD_STYLE );
    int32_T style_int = 0;
    if (style_arr && ymx_is_valid_int_scalar(style_arr)) {
        style_int = ymx_get_int_scalar(style_arr);
    } else {
        mexWarnMsgTxt("Invalid node style");
    }
    
    /* Get value */
    mxArray *value_arr = mxGetFieldByNumber( node, ind,
            YMX_NODE_FIELD_VALUE );
    
    /* Handle type */
    mxArray *type = mxGetFieldByNumber(node, ind, YMX_NODE_FIELD_TYPE);
    if (!type || !ymx_is_valid_int_scalar(type))
        ymx_error("Invalid node type");
    
    switch (ymx_get_int_scalar(type)) {
        case YMX_NODE_TYPE_SCALAR:
            ymx_dump_scalar( emitter,
                    value_arr,
                    tag_str,
                    anchor_str,
                    implicit_int,
                    style_int );
            break;
        case YMX_NODE_TYPE_SEQUENCE:
            ymx_dump_sequence( emitter,
                    value_arr,
                    tag_str,
                    anchor_str,
                    implicit_int,
                    style_int );
            break;
        case YMX_NODE_TYPE_MAPPING:
            ymx_dump_mapping( emitter,
                    value_arr,
                    tag_str,
                    anchor_str,
                    implicit_int,
                    style_int );
            break;
        case YMX_NODE_TYPE_ALIAS:
            ymx_dump_alias( emitter,
                    anchor_str );
            break;
        default:
            ymx_error("Invalid node type value");
    }
    
    if (tag_str)
        mxFree(tag_str);
    if (anchor_str)
        mxFree(anchor_str);
}

void ymx_dump_scalar(
        yaml_emitter_t *emitter,
        mxArray *value_arr,
        yaml_char_t *tag,
        yaml_char_t *anchor,
        int32_T implicit,
        int32_T style ) {
    
    if (!value_arr || !ymx_is_valid_string(value_arr))
        ymx_error("Invalid scalar value");
    
    yaml_char_t *value_str = (yaml_char_t *)mxArrayToString(value_arr);
    int plain_implicit = 0;
    int quoted_implicit = 0;
    switch (implicit) {
        case YMX_SCALAR_IMPLICIT_PLAIN:
            plain_implicit = 1;
            break;
        case YMX_SCALAR_IMPLICIT_QUOTED:
            quoted_implicit = 1;
            break;
    }
    
    yaml_event_t *event = &persistent_event;
    yaml_scalar_event_initialize( event,
            anchor,
            tag,
            value_str, strlen((char *)value_str),
            plain_implicit, quoted_implicit,
            style );
    ymx_emit(emitter, event);
    
    mxFree(value_str);
}

void ymx_dump_sequence(
        yaml_emitter_t *emitter,
        mxArray *value_arr,
        yaml_char_t *tag,
        yaml_char_t *anchor,
        int32_T implicit,
        int32_T style ) {
    yaml_event_t *event = &persistent_event;
    yaml_sequence_start_event_initialize( event,
            anchor,
            tag,
            implicit,
            style );
    ymx_emit(emitter, event);
    
    if (value_arr && !mxIsEmpty(value_arr)) {
        if ( !ymx_is_valid_node_array(value_arr)
                || mxGetNumberOfDimensions(value_arr) != 2
                || mxGetM(value_arr) != 1 )
            ymx_error("Invalid sequence node array");
        
        mwSize size = mxGetNumberOfElements(value_arr);
        mwIndex ind;
        for (ind = 0; ind < size; ind++) {
            ymx_dump_node(emitter, value_arr, ind);
        }
    }
    
    yaml_sequence_end_event_initialize(event);
    ymx_emit(emitter, event);
}

void ymx_dump_mapping(
        yaml_emitter_t *emitter,
        mxArray *value_arr,
        yaml_char_t *tag,
        yaml_char_t *anchor,
        int32_T implicit,
        int32_T style ) {
    yaml_event_t *event = &persistent_event;
    yaml_mapping_start_event_initialize( event,
            anchor,
            tag,
            implicit,
            style );
    ymx_emit(emitter, event);
    
    if (value_arr && !mxIsEmpty(value_arr)) {
        if ( !ymx_is_valid_node_array(value_arr)
                || mxGetNumberOfDimensions(value_arr) != 2
                || mxGetM(value_arr) != 2 )
            ymx_error("Invalid mapping node array");
        
        mwSize size = mxGetNumberOfElements(value_arr);
        mwIndex ind;
        for (ind = 0; ind < size; ind++) {
            ymx_dump_node(emitter, value_arr, ind);
        }
    }
    
    yaml_mapping_end_event_initialize(event);
    ymx_emit(emitter, event);
}

void ymx_dump_alias(
        yaml_emitter_t *emitter,
        yaml_char_t *anchor ) {
    yaml_event_t *event = &persistent_event;
    yaml_alias_event_initialize(event, anchor);
    ymx_emit(emitter, event);
}


/*
 * ymx_buffer functions
 *************************************************************************/

void ymx_buffer_init(ymx_buffer_t *buffer, size_t chunk_size) {
    buffer->chunk_size = chunk_size;
    buffer->head = mxMalloc(chunk_size + YMX_BUFFER_TAIL_SIZE);
    buffer->total_size = chunk_size;
    buffer->used_size = 0;
}

void ymx_buffer_delete(ymx_buffer_t *buffer) {
    if (buffer->head)
        mxFree(buffer->head);
    
    memset(buffer, 0, sizeof(ymx_buffer_t));
}

void ymx_buffer_append(ymx_buffer_t *buffer, 
        void *input, size_t size) {
    size_t current_space = buffer->total_size - buffer->used_size;
    if (size > current_space) {
        /* We need to make some room */
        int required_chunks = 1 + (size-current_space)/buffer->chunk_size;
        buffer->head = mxRealloc(buffer->head, buffer->total_size
                + (buffer->chunk_size * required_chunks)
                + YMX_BUFFER_TAIL_SIZE);
    }
    memcpy(buffer->head + buffer->used_size, input, size);
    buffer->used_size += size;
}

ymx_buffer_char_t *ymx_buffer_as_string(ymx_buffer_t *buffer) {
    memset(buffer->head + buffer->used_size,
            0, sizeof(ymx_buffer_char_t));
    return (ymx_buffer_char_t *) buffer->head;
}

static int ymx_buffer_write_handler(void *data,
        unsigned char *yaml_buffer, size_t size) {
    ymx_buffer_t *ymx_buffer = data;
    ymx_buffer_append(ymx_buffer, yaml_buffer, size);
    return 1;
}

/* int ymx_is_valid_node_item(const mxArray *node, mwIndex ind) {
//     ymx_node_t members;
//     members.type = mxGetFieldByNumber(node, ind,
//             YMX_NODE_FIELD_TYPE );
//     members.value = mxGetFieldByNumber(node, ind,
//             YMX_NODE_FIELD_VALUE );
//     members.tag = mxGetFieldByNumber(node, ind,
//             YMX_NODE_FIELD_TAG );
//     members.anchor = mxGetFieldByNumber(node, ind,
//             YMX_NODE_FIELD_ANCHOR );
//     members.implicit = mxGetFieldByNumber(node, ind,
//             YMX_NODE_FIELD_IMPLICIT );
//     members.style = mxGetFieldByNumber(node, ind,
//             YMX_NODE_FIELD_STYLE );
//     
//     // Require type as int scalar
//     if ( !members.type
//             || !ymx_is_valid_int_scalar(members.type) )
//         return 0;
//     
//     // Require implicit as int scalar
//     if ( !members.implicit
//             || !ymx_is_valid_int_scalar(members.implicit) )
//         return 0;
//     
//     // Require style as int scalar
//     if ( !members.style
//             || !ymx_is_valid_int_scalar(members.style) )
//         return 0;
//     
//     // If tag is given, require to be string
//     if ( members.tag
//             && mxGetNumberOfElements(members.tag) > 0
//             && !ymx_is_valid_string(members.tag) )
//         return 0;
//     
//     switch (ymx_get_int_scalar(members.type)) {
//         case YMX_NODE_TYPE_SCALAR:
//             // Require value as string
//             if ( !members.value
//                     || !ymx_is_valid_string(members.value) )
//                 return 0;
//             
//             // If anchor is given, require to be string
//             if ( members.anchor
//                     && mxGetNumberOfElements(members.anchor) > 0
//                     && !ymx_is_valid_string(members.anchor) )
//                 return 0;
//             
//             break;
//         case YMX_NODE_TYPE_SEQUENCE:
//             // Require value
//             if ( !members.value )
//                 return 0;
//             // Value must be empty, or 1-by-N node array
//             if ( mxGetNumberOfElements(members.value) > 0 && (
//                     mxGetNumberOfDimensions(members.value) > 2
//                     || mxGetM(members.value) != 1
//                     || !ymx_is_valid_node_array(members.value) ) )
//                 return 0;
//             
//             // If anchor is given, require to be string
//             if ( members.anchor
//                     && mxGetNumberOfElements(members.anchor) > 0
//                     && !ymx_is_valid_string(members.anchor) )
//                 return 0;
//             
//             break;
//         case YMX_NODE_TYPE_MAPPING:
//             // Require value
//             if ( !members.value )
//                 return 0;
//             // Value must be empty, or 2-by-N node array
//             if ( mxGetNumberOfElements(members.value) > 0 && (
//                     mxGetNumberOfDimensions(members.value) > 2
//                     || mxGetM(members.value) != 2
//                     || !ymx_is_valid_node_array(members.value) ) )
//                 return 0;
//             
//             // If anchor is given, require to be string
//             if ( members.anchor
//                     && mxGetNumberOfElements(members.anchor) > 0
//                     && !ymx_is_valid_string(members.anchor) )
//                 return 0;
//             
//             break;
//         case YMX_NODE_TYPE_ALIAS:
//             // Require anchor as nonempty string
//             if ( !members.anchor
//                     || !mxGetNumberOfElements(members.anchor) > 0
//                     || !ymx_is_valid_string(members.anchor) )
//                 return 0;
//             
//             break;
//         default:
//             // Any other type code is invalid
//             return 0;
//     }
//     
//     return 1;
// } */

/* Determine whether an individual document in an array is valid. Assumes
 * ymx_is_valid_doc_array has already been called and returned 1, or
 * the document array is otherwise known to be valid. Returns 1 iff each
 * member of the document struct is valid; otherwise returns 0. Does not
 * traverse the full node tree to assess its validity; simply verifies that
 * the top-level root has valid structure with ymx_is_valid_node_array.
 */
/* int ymx_is_valid_doc_item(const mxArray *doc_array, mwIndex ind) {
//     ymx_document_t members;
//     members.root = mxGetFieldByNumber(doc_array, ind,
//             YMX_DOC_FIELD_ROOT );
//     members.tagdirs = mxGetFieldByNumber(doc_array, ind,
//             YMX_DOC_FIELD_TAGDIRS );
//     members.version = mxGetFieldByNumber(doc_array, ind,
//             YMX_DOC_FIELD_VERSION );
//     members.start_implicit = mxGetFieldByNumber(doc_array, ind,
//             YMX_DOC_FIELD_START_IMPLICIT );
//     members.end_implicit = mxGetFieldByNumber(doc_array, ind,
//             YMX_DOC_FIELD_END_IMPLICIT );
//     
//     if ( !members.root
//             || !mxGetNumberOfElements(members.root) == 1
//             || !ymx_is_valid_node_array(members.root) )
//         return 0;
//     
//     if ( members.tagdirs
//             && mxGetNumberOfElements(members.tagdirs) > 0
//             && !ymx_is_valid_tagdirs_array(members.tagdirs) )
//         return 0;
//     
//     if ( members.version
//             && mxGetNumberOfElements(members.version) > 0
//             && !ymx_is_valid_version_array(members.version) )
//         return 0;
//     
//     if ( !members.start_implicit
//             || !ymx_is_valid_int_scalar(members.start_implicit) )
//         return 0;
//     
//     if ( !members.end_implicit
//             || !ymx_is_valid_int_scalar(members.end_implicit) )
//         return 0;
//     
//     return 1;
// } */
