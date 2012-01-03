function yaml_data = yaml_load(yaml_stream)

yaml_doc = yaml_mex('load', yaml_stream);
yaml_data = yaml_simple_construct(yaml_doc.root);
