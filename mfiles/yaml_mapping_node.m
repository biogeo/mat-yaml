function node = yaml_mapping_node(tag, anchor, implicit, style)

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
