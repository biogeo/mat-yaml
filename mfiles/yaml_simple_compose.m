function yaml_doc = yaml_simple_compose(data, node_only)
% Performs very simplistic composition of Matlab native data types into a
% YAML document structure (with some stylistic indications to enhance
% prettiness). There are a number of assumptions and limitations:
% ---
% Dimensionality: >
%   Simple composition does nothing to respect the dimensionality of an
%   array; all non-scalars are dumped as a sequence (1-dimensional).
% Matlab types:
%   Numeric arrays: >
%     All numeric arrays are dumped as sequences of !!float elements.
%   Logical arrays: >
%     Logical arrays are dumped as sequences of !!bool elements in
%     canonical true/false form.
%   Character arrays: >
%     Character arrays are dumped as !!str, quoted style.
%   Cell arrays: >
%     Cell arrays are dumped as sequences, no problem.
%   Structs: >
%     Structs are dumped as mappings, with the fields being the keys and
%     the struct values as the mapping values. Struct arrays are just
%     sequences of mappings.
% ...
% For the future, some nice ad-hoc Matlab-specific tags to specify
% dimensionality might be a good idea. That is, create a schema for Matlab
% array types. For now, this will suffice for basic functionality.

if ~exist('node_only','var') || isempty(node_only)
    node_only = false;
end

if ischar(data)
    node = yaml_scalar_node(data, 'tag:yaml.org,2002:str', [], 2);
elseif iscell(data)
    node = yaml_sequence_node([],[],1);
    seq_items = cell(1,numel(data));
    for i=1:numel(seq_items)
        seq_items{i} = yaml_simple_compose(data{i}, true);
    end
    node.value = [seq_items{:}];
elseif isempty(data)
    node = yaml_scalar_node('', 'tag:yaml.org,2002:null', [], 1);
elseif isnumeric(data)
    if isscalar(data)
        node = yaml_scalar_node(yaml_num2str(data), ...
            'tag:yaml.org,2002:float', [], 1);
    else
        node = yaml_sequence_node([],[],1,2);
        node.value = arrayfun(@(x)(yaml_scalar_node(yaml_num2str(x), ...
            'tag:yaml.org,2002:float', [], 1)), data(:)');
    end
elseif islogical(data)
     tfstr = {'false', 'true'};
    if isscalar(data)
        node = yaml_scalar_node(tfstr{data+1}, ...
            'tag:yaml.org,2002:bool', [], 1);
    else
        node = yaml_sequence_node([],[],1,2);
        node.value = yaml_scalar_node(tfstr(data(:)'+1), ...
            'tag:yaml.org,2002:bool', [], 1);
    end
elseif isstruct(data)
    if isscalar(data)
        node = yaml_mapping_node([],[],1);
        fnames = fieldnames(data)';
        keys = cellfun( ...
            @(s)yaml_scalar_node(s, 'tag:yaml.org,2002:str',[],2), ...
            fnames);
        values = cellfun(@(x)yaml_simple_compose(data.(x), true), fnames);
        node.value = [keys;values];
    else
        node = yaml_sequence_node([],[],1);
        node.value = arrayfun(@(s)yaml_simple_compose(s,true), data(:)');
    end
else
    error('yaml_simple_compose:unsupportedType', ...
        'Can''t compose this datatype.');
end

if node_only
    yaml_doc = node;
else
    yaml_doc = yaml_document(node,[],[],true,true);
end


function str = yaml_num2str(num)
if num == Inf
    str = '.Inf';
elseif num == -Inf
    str = '-.Inf';
elseif isnan(num)
    str = '.NaN';
else
    str = num2str(num);
end