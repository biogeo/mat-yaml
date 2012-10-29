function yaml_stream = yaml_file_dump(filename, data)
% yaml_file_dump  Dump Matlab data to a YAML file
% Usage:
%     yaml_file_dump(filename, data)
%     yaml_stream = yaml_file_dump(...)
% Accepts a basic Matlab object (numeric, char, logical, cell, and struct)
% as "data" and emits it as a YAML document, stored in a new YAML file
% "filename". More complex Matlab datatypes (eg, classes) cannot be dumped.
% The stream may optionally also be returned as an output string.
% Matlab data is represented in YAML in the following way:
%
% A non-scalar object of any type except char:
%     will be repesented as a YAML sequence of its elements. Array
%     dimensionality and type information are not preserved explicitly, so
%     beware if you plan to load the data back into Matlab.
% A char array:
%     will be represented as a YAML string.
% A numeric scalar:
%     will be represented as a number
% A logical scalar:
%     will be represented as a bool (true or false).
% A struct scalar:
%     will be represented as a YAML mapping, with the field names being
%     keys and the field values being the corresponding values.
% A cell array:
%     will be represented as a YAML sequence, even if it only one element.
%
% Note that because Matlab's arrays are typed, and YAML's sequence is
% untyped, the full structure of the Matlab data is not preserved when
% dumping to YAML using this function. For example, the command:
%     yaml_load(yaml_dump(1:10))
% will not produce the numeric array 1:10, but instead the equivalent of
%     mat2cell(1:10,1,ones(1,10))
% (ie, each element in the array becomes a scalar element in a cell array).
% In many cases it will be easy to recover the original data structure if
% its types are known.
%
% This is a convenience function which calls yaml_dump to compose the YAML
% document and fwrite to write it to a file.

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

my_yaml_stream = yaml_dump(data);
fid = fopen(filename, 'w');
try
    fwrite(fid, my_yaml_stream);
    fclose(fid);
catch e
    if ismember(fid, fopen('all'))
        fclose(fid);
    end
    rethrow(e);
end
if nargout
    yaml_stream = my_yaml_stream;
end