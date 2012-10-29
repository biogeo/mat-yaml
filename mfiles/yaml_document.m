function doc = yaml_document(root, version, tagdirs, start_implicit, end_implicit)
% yaml_document  Compose a document node for yaml_mex
% Usage:
%     doc=yaml_document(root,version,tagdirs,start_implicit,end_implicit)
% Composes a YAML document node representation suitable for passing to
% yaml_mex (see "help yaml_mex" for more information). The arguments
% version, tagdirs, start_implicit, and end_implicit are optional.
%
% doc is a struct with the following fields, taken from yaml_document's
% inputs (or given default values):
%
%     root:           The root node of the document. Should be a YAML node,
%                     such as output by yaml_scalar_node,
%                     yaml_sequence_node, or yaml_mapping_node.
%     version:        A 1-by-2 numeric array specifying the %YAML version
%                     directive, as [major, minor] version number, or an
%                     empty array for no version directive. yaml_document
%                     will cast the array to int32.
%     tagdirs:        A struct array containing the %TAG tag directives,
%                     if any. Must have the fields 'handle' and 'prefix',
%                     each holding a string. Each element in the array
%                     corresponds to a tag directive of the form:
%                         %TAG <handle> <prefix>
%                     Or, an empty array for no tag directives.
%     start_implicit: A logical scalar indicating whether the document
%                     start is implicit (true) or explicit (false). If the
%                     start is explicit, the document begins with "---".
%     end_implicit:   A logical scalar indicating whether the document end
%                     is implicit (true) or explicit (false). If the end is
%                     explicit, the document ends with "...".
%
% Note that libyaml may not honor all presentation style requests.

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

if ~exist('version','var') || isempty(version)
    version = [];
elseif numel(version) ~= 2
    error('yaml_document:badVersion', ...
        'version must be a 2-element numeric');
else
    version = int32(version);
end

if ~exist('tagdirs','var') || isempty(tagdirs)
    tagdirs = [];
end

if ~exist('start_implicit','var') || isempty(start_implicit)
    start_implicit = false;
elseif ~isscalar(start_implicit)
    error('yaml_document:badStartImplicit', ...
        'start_implicit must be a scalar');
else
    start_implicit = logical(start_implicit);
end

if ~exist('end_implicit','var') || isempty(end_implicit)
    end_implicit = false;
elseif ~isscalar(end_implicit)
    error('yaml_document:badEndImplicit', 'end_implicit must be a scalar');
else
    end_implicit = logical(end_implicit);
end

doc = struct( 'root', root, ...
    'version', version, ...
    'tagdirs', tagdirs, ...
    'start_implicit', start_implicit, ...
    'end_implicit', end_implicit );
