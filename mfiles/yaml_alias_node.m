function node = yaml_alias_node(anchor)
% yaml_alias_node  Compose an alias node for yaml_mex
% Usage:
%     node = yaml_alias_node(anchor)
% Composes a YAML alias node representation suitable for use with yaml_mex
% (see "help yaml_mex" for more information).
%
% node is a struct with the following fields:
%
%     type:     The value int32(4) (to denote an alias node).
%     value:    The empty matrix []. This field is ignored for alias nodes.
%     tag:      The empty string ''. This field is ignored for alias nodes.
%     anchor:   The anchor string supplied as an input; labels the anchor
%               that the alias references.
%     implicit: The value int32(0). This field is ignored for alias nodes.
%     style:    The value int32(0). This field is ignored for alias nodes.

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

if ~(ischar(anchor) && size(anchor,1) <= 1) || iscellstr(anchor)
    error('yaml_alias_node:badAnchor', 'anchor must be a string');
end

node = struct( 'type', int32(4), ...
    'value', [], ...
    'tag', '', ...
    'anchor', anchor, ...
    'implicit', int32(0), ...
    'style', int32(0) );