#include "mex.h"
#include <yaml.h>
#include "yaml_mex_util.h"

void command_load( int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[] );
void command_dump( int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[] );
void command_help( int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[] );

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] )
{
    ymx_debug_msg("Entering mexFunction\n");
    mexAtExit(ymx_persistent_cleanup);
    
    if (nrhs == 0) {
        command_help(nlhs, plhs, 0, NULL);
        return;
    }
    if (!mxIsChar(prhs[0]) || mxGetM(prhs[0]) != 1)
    {
        mexErrMsgTxt("First input must be a string.");
    }
    char *command = mxArrayToString(prhs[0]);
    if (strcmp(command, "load") == 0) {
        command_load(nlhs, plhs, nrhs-1, prhs+1);
    } else if (strcmp(command, "dump") == 0) {
        command_dump(nlhs, plhs, nrhs-1, prhs+1);
    } else if (strcmp(command, "help") == 0) {
        command_help(nlhs, plhs, nrhs-1, prhs+1);
    } else {
        command_help(nlhs, plhs, 0, NULL);
    }
    ymx_debug_msg("plhs: %d\n", plhs);
    ymx_debug_msg("plhs[0]: %d\n", plhs[0]);
    mxFree(command);
    ymx_debug_msg("Exiting mexFunction\n");
}

void command_help( int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[] ) {
    mexPrintf("Usage:\n");
    mexPrintf("    doc = yaml_mex('load', yaml_str)\n");
    mexPrintf("    yaml_str = yaml_mex('dump', doc)\n");
    mexPrintf("Type \"help yaml_mex\" for more information.\n");
}

void command_load( int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[] ) {
    ymx_debug_msg("Entering command_load\n");
    if (nrhs != 1 || !mxIsChar(prhs[0]) || mxGetM(prhs[0]) > 1) {
        mexErrMsgTxt("'load' requires 1 additional string input.");
    }
    /*plhs = mxCalloc(1, sizeof(mxArray **));*/
    plhs[0] = ymx_load_stream(prhs[0]);
    /*mexCallMATLAB(0, NULL, 1, plhs[0], "disp");*/
    ymx_debug_msg("Exiting command_load\n");
}

void command_dump( int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[] ) {
    if (nrhs != 1) {
        mexErrMsgTxt("'dump' requires 1 additional input.");
    }
    plhs[0] = ymx_dump_stream(prhs[0]);
}
