function yaml_stream = yaml_dump(data)

yaml_doc = yaml_simple_compose(data);
yaml_stream = yaml_mex('dump', yaml_doc);