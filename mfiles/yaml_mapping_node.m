function node = yaml_mapping_node(tag, anchor, implicit, style)
% yaml_mapping_node  Compose a mapping node for yaml_mex
% Usage:
%     node = yaml_mapping_node(tag, anchor, implicit, style)
% Composes a YAML mapping node representation suitable for use with
% yaml_mex (see "help yaml_mex" for more information). All arguments are
% optional.
%
% node is a struct with the following fields, taken from the input
% arguments (or filled with default values):
%
%     type:     The value int32(3) (to denote a mapping node).
%     value:    An empty matrix. To set the value, fill with a 2-by-N array
%               of YAML nodes, as created by yaml_scalar_node,
%               yaml_sequence_node, yaml_mapping_node, and yaml_alias_node.
%               Each column of the matrix represents one key-value pair in
%               the mapping, with the first row representing keys and the
%               second row values.
%     tag:      A YAML tag descriptor string, taken from the input
%               argument. Defaults to 'tag:yaml.org,2002:map'.
%     anchor:   A string labeling the node with an anchor, or empty for no
%               anchor (default); taken from the input argument.
%     implicit: A presentation detail, defining how the tag is shown in the
%               YAML document. Taken from the input, must be 0 or false
%               (explicit), or 1 or true (implicit). Will be cast to int32.
%     style:    A number defining the presentation style of the node. Must
%               be one of the following:
%               0: Any mapping style (let libyaml decide)
%               1: Block mapping style
%               2: Flow mapping style
%               Will be cast to int32.
%
% Note that libyaml may not honor all presentation style requests, for
% example a flow style for a mapping containing block style nodes.

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

if ~exist('tag','var') || isempty(tag)
    tag = 'tag:yaml.org,2002:map';
elseif ~(ischar(tag) && size(tag,1) <= 1) || iscellstr(tag)
    error('yaml_mapping_node:badTag', 'tag must be a string');
end

if ~exist('anchor','var') || isempty(anchor)
    anchor = [];
elseif ~(ischar(anchor) && size(anchor,1) <= 1) || iscellstr(anchor)
    error('yaml_mapping_node:badAnchor', 'anchor must be a string');
end

if ~exist('implicit','var') || isempty(implicit)
    implicit = int32(0);
elseif ~isscalar(implicit)
    error('yaml_mapping_node:badImplicit', 'implicit must be a scalar');
else
    implicit = int32(implicit);
end

if ~exist('style','var') || isempty(style)
    style = int32(0);
elseif ~isscalar(style)
    error('yaml_mapping_node:badStyle', 'style must be a scalar');
else
    style = int32(style);
end

node = struct( 'type', int32(3), ...
    'value', [], ...
    'tag', tag, ...
    'anchor', anchor, ...
    'implicit', implicit, ...
    'style', style );
