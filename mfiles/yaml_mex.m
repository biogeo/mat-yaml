% yaml_mex    A MEX-file for processing YAML
% Usage:
%     doc = yaml_mex('load', str)
%     str = yaml_mex('dump', doc)
%
% yaml_mex is a high-level interface to the libyaml YAML processor. It
% exposes to Matlab a partially-constructed representation of a YAML
% document that includes some presentation details, such as anchor names
% and flow styles. yaml_mex can load YAML streams into this format, and can
% dump this format into YAML streams. In dumping, yaml_mex strictly
% requires structures of the same format that are produced by loading.
% Other Matlab code should handle the translation between this partial
% representation and fully native Matlab data structures (numeric arrays,
% cell arrays, structs, etc.)
%
% For most use cases, it will be easier to use yaml_load and yaml_dump to
% interface with YAML files. However, these functions cannot deal properly
% with aliases or mapping keys which are not valid Matlab struct field
% names.
%
% A YAML stream is represented as an array of structs, each representing a
% document in the stream. Documents have must these fields:
%               root: The root node of the document, see below.
%            version: A 1-by-2 int32 array containing the %YAML version
%                     directive. version(1) is the major version number,
%                     version(2) is the minor number. Empty (of any class)
%                     if there is no version directive.
%            tagdirs: A struct array containing the %TAG tag directives,
%                     if any. Must have the fields 'handle' and 'prefix',
%                     each holding a string. Each element in the array
%                     corresponds to a tag directive of the form:
%                         %TAG <handle> <prefix>
%                     Empty (of any class) if there are no tag directives.
%     start_implicit: A logical scalar indicating whether the document
%                     start is implicit (true) or explicit (false). If the
%                     start is explicit, the document begins with "---".
%       end_implicit: A logical scalar indicating whether the document end
%                     is implicit (true) or explicit (false). If the end is
%                     explicit, the document ends with "...".
%
% The document is represented as a tree with its root at <doc>.root. With
% aliases, YAML documents may in fact be cyclic graphs, but Matlab does not
% offer a native data type with this capability, so aliases are represented
% as just a type of leaf on the tree. Nodes must have these fields:
%         type: An int32 scalar. Its values indicate the type as follows:
%               1: scalar
%               2: sequence
%               3: mapping
%               4: alias
%        value: This depends on the type of node:
%                 scalar: Must be a string; gives the value of the scalar.
%               sequence: A 1-by-N array of nodes, or empty (of any class).
%                         Each element in the array is an item in the
%                         sequence.
%                mapping: A 2-by-N array of nodes, or empty (of any class).
%                         Each column in the array represents one key-value
%                         pair in the mapping, with the first row
%                         representing keys and the second row values.
%                  alias: Not used.
%          tag: Must be a string, or empty (of any class). Not used for
%               aliases.
%       anchor: Must be a string, or empty (of any class). May not be empty
%               for aliases.
%     implicit: An int32 scalar describing whether the tag is implicit. For
%               sequences and mappings, it is just a boolean. For scalars,
%               it can be 0 (explicit), 1 (implicit plain-style), or 2
%               (implicit quoted-style). For aliases, should be set, but
%               its value is ignored.
%        style: An int32 scalar describing the presentation style of the
%               node. The meaning of its values depends on the type of
%               node:
%                 scalar: 0: Any scalar style (let libyaml decide)
%                         1: Plain scalar style
%                         2: Single-quoted scalar style
%                         3: Double-quoted scalar style
%                         4: Literal scalar style
%                         5: Folded scalar style
%               sequence: 0: Any sequence style (let libyaml decide)
%                         1: Block sequence style
%                         2: Flow sequence style
%                mapping: 0: Any mapping style (let libyaml decide)
%                         1: Block mapping style
%                         2: Flow mapping style
%                  alias: A value should be set, but it will be ignored.
% 
% When dumping a stream, there is no guarantee that presentation style
% requests will be honored.
%
% To ensure that yaml_mex is supplied with the proper YAML document
% representation, it is strongly recommended that you use these functions
% to compose the input to yaml_mex('dump'):
%     yaml_document:      Compose the document root
%     yaml_scalar_node:   Compose a scalar node with a supplied value
%     yaml_sequence_node: Compose an empty sequence node
%     yaml_mapping_node:  Compose an empty mapping node
%     yaml_alias_node:    Compose an alias node with a supplied anchor
% These functions perform the proper error checking to ensure that the
% resulting nodes are suitable for passing to yaml_mex.

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

% This code should only be executed if the yaml_mex MEX-file is not
% available.
error('yaml_mex:NoMex', ['yaml_mex has not been built, or is not on ' ...
    'the Matlab path. See the README file for instructions on ' ...
    'how to build yaml_mex.']);
