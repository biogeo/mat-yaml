function doc = yaml_document(root, version, tagdirs, start_implicit, end_implicit)

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
