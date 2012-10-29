function node = yaml_scalar_node(value, tag, anchor, implicit, style)
% yaml_scalar_node  Compose a scalar node for yaml_mex
% Usage:
%     node=yaml_scalar_node(value,tag,anchor,implicit,style)
% Composes a YAML scalar node representation suitable for use with yaml_mex
% (see "help yaml_mex" for more information). The arguments tag, anchor,
% implicit, and style are optional.
%
% node is a struct with the following fields, taken from yaml_scalar_node's
% inputs (or given default values):
%
%     type:     The value int32(1) (to denote a scalar node).
%     value:    A string giving the value of the scalar node, taken from
%               the input argument. Note that values must already be cast
%               to their string representations (e.g., 256 should become
%               '256').
%     tag:      A YAML tag descriptor string, taken from the input
%               argument. Defaults to 'tag:yaml.org,2002:str'.
%     anchor:   A string labeling the node with an anchor, or empty for no
%               anchor (default); taken from the input argument.
%     implicit: A presentation detail, defining how the tag is shown in the
%               YAML document. Taken from the input, must be 0 (explicit
%               tag), 1 (implicit plain-style), or 2 (implicit
%               quoted-style). Will be cast to int32.
%     style:    A number defining the presentation style of the node. Must
%               one of the following:
%               0: Any scalar style (let libyaml decide)
%               1: Plain scalar style
%               2: Single-quoted scalar style
%               3: Double-quoted scalar style
%               4: Literal scalar style
%               5: Folded scalar style
%               Will be cast to int32.
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

if isempty(value)
    value = '';
elseif ~(ischar(value) && size(value,1) <= 1) || iscellstr(value)
    error('yaml_scalar_node:badValue', 'value must be a string');
end

if ~exist('tag','var') || isempty(tag)
    tag = 'tag:yaml.org,2002:str';
elseif ~(ischar(tag) && size(tag,1) <= 1) || iscellstr(tag)
    error('yaml_scalar_node:badTag', 'tag must be a string');
end

if ~exist('anchor','var') || isempty(anchor)
    anchor = [];
elseif ~(ischar(anchor) && size(anchor,1) <= 1) || iscellstr(anchor)
    error('yaml_scalar_node:badAnchor', 'anchor must be a string');
end

if ~exist('implicit','var') || isempty(implicit)
    implicit = int32(0);
elseif ~isscalar(implicit)
    error('yaml_scalar_node:badImplicit', 'implicit must be a scalar');
else
    implicit = int32(implicit);
end

if ~exist('style','var') || isempty(style)
    style = int32(0);
elseif ~isscalar(style)
    error('yaml_scalar_node:badStyle', 'style must be a scalar');
else
    style = int32(style);
end

node = struct( 'type', int32(1), ...
    'value', value, ...
    'tag', tag, ...
    'anchor', anchor, ...
    'implicit', implicit, ...
    'style', style );
