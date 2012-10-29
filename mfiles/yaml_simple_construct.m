function data = yaml_simple_construct(yaml_node)
% yaml_simple_construct  Construct Matlab native data from YAML nodes
% Usage:
%     data = yaml_simple_construct(yaml_node)
% Peforms a very simplistic construction of Matlab data types from a YAML
% document or node, structured as output by yaml_mex (see "help yaml_mex").
% This construction will attempt to follow the YAML 1.2 spec (although
% libyaml currently only supports YAML 1.1, and will produce an error when
% reading a document with the "%YAML 1.2" directive), with some
% limitations:
% ---
% Aliases:
%   -> Matlab native data types don't implement references, so aliases
%      won't work. These could be implemented with a user-defined handle
%      subclass, but then it's no longer really a "simple" construction.
% Mappings:
%   -> The closest thing Matlab has to a native mapping type is a scalar
%      struct. This is limited to keys that are valid Matlab identifiers.
%      Therefore attempting to construct any other sort of mapping will
%      fail. A user-defined mapping class could be implemented, but again,
%      this wouldn't really be "simple" any more.
% Sequences:
%   -> All sequences will be constructed as cell arrays. If the sequence is
%      intended to represent, e.g., a numeric vector, this should be easy
%      enough for downstream code to convert (eg, [data{:}]).
% ...
% Scalars are resolved according to the YAML core tags.

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

% If we received a document, get its root.
if isfield(yaml_node, 'root')
    yaml_node = yaml_node.root;
end

switch yaml_node.type
    case 1 % scalar
        data = construct_scalar(yaml_node.value, yaml_node.tag);
    case 2 % sequence
        data = cell(size(yaml_node.value));
        for i=1:numel(yaml_node.value)
            data{i} = yaml_simple_construct(yaml_node.value(i));
        end
    case 3 % mapping
        keys = {yaml_node.value(1,:).value};
        values = cell(size(keys));
        for i=1:size(yaml_node.value,2)
            values{i} = {yaml_simple_construct(yaml_node.value(2,i))};
        end
        data_arg = [keys;values];
        try
            data = struct(data_arg{:});
        catch err
            if strcmp(err.identifier,'MATLAB:Cell2Struct:InvalidFieldName')
                error('yaml_simple_construct:mappingKey', ...
                    'Unable to convert mapping key to struct field.');
            end
        end
    case 4 % alias
        error('yaml_simple_construct:aliasType', ...
            'Cannot construct alias nodes.');
    otherwise
        error('yaml_simple_construct:unknownType', ...
            'Unrecognized node type.');
end

function data = construct_scalar(value, tag)
if ismember(tag, {'!', 'tag:yaml.org,2002:str'})
    data = value;
    return;
end

nonspecific = isempty(tag) || strcmp(tag,'?');

% Is this a null item?
if strcmp(tag,'tag:yaml.org,2002:null') || nonspecific
    match = isempty(value) || ismember(value, {'null','Null','NULL','~'});
    if match
        data = [];
        return;
    elseif ~nonspecific
        error('yaml_simple_construct:tagValueMismatch', ...
            'Scalar value didn''t match its tag!');
    end
end

% Is this a bool item?
if strcmp(tag,'tag:yaml.org,2002:bool') || nonspecific
    isTrue = ismember(value, {'true','True','TRUE'});
    isFalse = ismember(value, {'false','False','FALSE'});
    if isTrue || isFalse
        data = isTrue;
        return;
    elseif ~nonspecific
        error('yaml_simple_construct:tagValueMismatch', ...
            'Scalar value didn''t match its tag!');
    end
end

% Is this an int item?
if strcmp(tag, 'tag:yaml.org,2002:int') || nonspecific
    start = regexp(value, '^[-+]?[0-9]+$', 'once'); % Decimal
    if ~isempty(start)
        data = str2double(value);
        return;
    end
    [start, token] = regexp(value, '^0x([0-9a-fA-F]+)$', ...
        'start', 'tokens', 'once'); % Hexadecimal
    if ~isempty(start)
        data = hex2dec(token{1});
        return;
    end
    [start, token] = regexp(value, '^0o([0-7]+)$', ...
        'start', 'tokens', 'once'); % Octal
    if ~isempty(start)
        data = base2dec(token{1}, 8);
        return;
    end
    if ~nonspecific
        error('yaml_simple_construct:tagValueMismatch', ...
            'Scalar value didn''t match its tag!');
    end
end

% Is this a float item?
if strcmp(tag,'tag:yaml.org,2002:float') || nonspecific
    start = regexp(value, ...
        '^[-+]?(\.[0-9]+|[0-9]+(\.[0-9]*)?)([eE][-+]?[0-9]+)?$', ...
        'start', 'once');
    if ~isempty(start)
        data = str2double(value);
        return;
    end
    [start, token] = regexp(value, ...
        '^([-+]?)(?:\.inf|\.Inf|\.INF)$', ...
        'start', 'tokens', 'once');
    if ~isempty(start)
        if strcmp(token,'-')
            data = -Inf;
        else
            data = Inf;
        end
        return;
    end
    if ismember(value, {'.nan','.NaN','.NAN'})
        data = NaN;
        return;
    end
    if ~nonspecific
        error('yaml_simple_construct:tagValueMismatch', ...
            'Scalar value didn''t match its tag!');
    end
end

% We've run out of tags. If this is a nonspecific tag, no problem; we
% resolve to string. Otherwise, we failed to resolve the tag.
if ~nonspecific
    error('yaml_simple_construct:unknownTag', ...
        'Unrecognized tag');
end

data = value;
