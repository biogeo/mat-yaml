#ifndef YAML_MEX_UTIL_H
#define YAML_MEX_UTIL_H

#include "mex.h"
#include <yaml.h>
#include <string.h>

#ifdef YMX_DEBUG
#define ymx_debug_msg(...) mexPrintf(__VA_ARGS__)
#else
#define ymx_debug_msg(...)
#endif

/*
 * Main interface functions
 *************************************************************************/

mxArray *ymx_load_stream(const mxArray *yaml_stream);
mxArray *ymx_dump_stream(const mxArray *docs_array);
void ymx_persistent_cleanup();

/*
 * Types and constants for creating specialized mxArray structs
 *************************************************************************/

extern const char *YMX_NODE_TYPE_STRS[];
typedef enum ymx_node_type_e {
    YMX_NODE_TYPE_NONE,
    YMX_NODE_TYPE_SCALAR,
    YMX_NODE_TYPE_SEQUENCE,
    YMX_NODE_TYPE_MAPPING,
    YMX_NODE_TYPE_ALIAS
} ymx_node_type_t;

extern const char *YMX_NODE_FIELD_STRS[];
typedef enum ymx_node_field_e {
    YMX_NODE_FIELD_TYPE,
    YMX_NODE_FIELD_VALUE,
    YMX_NODE_FIELD_TAG,
    YMX_NODE_FIELD_ANCHOR,
    YMX_NODE_FIELD_IMPLICIT,
    YMX_NODE_FIELD_STYLE,
    YMX_NODE_NUM_FIELDS
} ymx_node_field_t;

extern const char *YMX_DOC_FIELD_STRS[];
typedef enum ymx_doc_field_e {
    YMX_DOC_FIELD_ROOT,
    YMX_DOC_FIELD_VERSION,
    YMX_DOC_FIELD_TAGDIRS,
    YMX_DOC_FIELD_START_IMPLICIT,
    YMX_DOC_FIELD_END_IMPLICIT,
    YMX_DOC_NUM_FIELDS
} ymx_doc_field_t;

extern const char *YMX_TAGDIR_FIELD_STRS[];
typedef enum ymx_tagdir_field_e {
    YMX_TAGDIR_FIELD_HANDLE,
    YMX_TAGDIR_FIELD_PREFIX,
    YMX_TAGDIR_NUM_FIELDS
} ymx_tagdir_field_t;

typedef enum ymx_scalar_implicit_flag_e {
    YMX_SCALAR_IMPLICIT_FALSE,
    YMX_SCALAR_IMPLICIT_PLAIN,
    YMX_SCALAR_IMPLICIT_QUOTED
} ymx_scalar_implicit_flag_t;

#endif /* #ifndef YAML_MEX_UTIL_H */