% yaml_mex    A mex-file for processing YAML
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