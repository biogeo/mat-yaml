function data = yaml_file_load(filename)
% yaml_load  Load data from a YAML file
% Usage:
%     data = yaml_file_load(filename)
% Loads a YAML stream from a file and constructs it to native Matlab data.
%
% YAML mappings:
%     are constructed as Matlab structs, subject to the constraint that the
%     mappings' keys must be valid Matlab struct field names. If they are
%     not, yaml_load will fail. (This data can still be read into Matlab
%     using yaml_mex, but you will have to handle the YAML document
%     structure yourself.)
% YAML sequences:
%     are constructed as Matlab cell arrays, 1-by-M. Note that this is done
%     independently of the datatypes of the sequence elements, meaning that
%     a sequence of all numbers will still be constructed as a cell array
%     rather than a numeric array.
% YAML strings:
%     are constructed as char arrays.
% YAML ints and floats:
%     are both constructed as doubles.
% YAML bools:
%     are constructed as logicals.
% YAML nulls:
%     are constructed as empty arrays ([]).
% YAML alias nodes:
%     will cause yaml_load to fail, as Matlab lacks a native reference
%     type. (These documents can still be read by yaml_mex, but you'll have
%     to handle the document structure yourself.)
%
% This is just a convenience function which calls fileread to load the YAML
% file and yaml_load to construct the native representation.

% Copyright (c) 2011 Geoffrey Adams
% 
% Permission is hereby granted, free of charge, to any person obtaining a
% copy of this software and associated documentation files
% (the "Software"), to deal in the Software without restriction, including
% without limitation the rights to use, copy, modify, merge, publish,
% distribute, sublicense, and/or sell copies of the Software, and to
% permit persons to whom the Software is furnished to do so, subject to the
% following conditions:
% 
% The above copyright notice and this permission notice shall be included
% in all copies or substantial portions of the Software.
% 
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
% OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
% MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
% NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
% DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
% OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
% USE OR OTHER DEALINGS IN THE SOFTWARE.

yaml_stream = fileread(filename);
data = yaml_load(yaml_stream);
