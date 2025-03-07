# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
	'conditions': [
		['OS!="win"', {
		  'variables': {
			'config_h_dir':
			  '.',  # crafted for gcc/linux.
		  },
		  'target_defaults': {
			 'cflags': [ '-std=c++0x', '-fPIC', '-fexceptions', '-fpermissive', '-Wc++0x-compat' ],
		  }
		}, {  # else, OS=="win"
		  'variables': {
			'config_h_dir':
			  'vsprojects',  # crafted for msvc.
		  },
		  'target_defaults': {
			'msvs_disabled_warnings': [
			  4018,  # signed/unsigned mismatch in comparison
			  4244,  # implicit conversion, possible loss of data
			  4355,  # 'this' used in base member initializer list
			  4267,  # size_t to int truncation
			  4291,  # no matching operator delete for a placement new
			],
			'defines!': [
			  'WIN32_LEAN_AND_MEAN',  # Protobuf defines this itself.
			],
		  },
		}]
	],
	'targets': [
	# The "lite" lib is about 1/7th the size of the heavy lib,
	# but it doesn't support some of the more exotic features of
	# protobufs, like reflection.  To generate C++ code that can link
	# against the lite version of the library, add the option line:
	#
	#   option optimize_for = LITE_RUNTIME;
	#
	# to your .proto file.
	{
	  'target_name': 'protobuf_lite',
	  'type': '<(component)',
	  #'toolsets': ['host', 'target'],
	  'includes': [
		'protobuf_lite.gypi',
	  ],
	  # Required for component builds. See http://crbug.com/172800.
	  'defines': [
		'LIBPROTOBUF_EXPORTS',
		'PROTOBUF_USE_DLLS',
	  ],
	  'direct_dependent_settings': {
		'defines': [
		  'PROTOBUF_USE_DLLS',
		],
	  },
	},
	# This is the full, heavy protobuf lib that's needed for c++ .protos
	# that don't specify the LITE_RUNTIME option.  The protocol
	# compiler itself (protoc) falls into that category.
	#
	# DO NOT LINK AGAINST THIS TARGET IN CHROME CODE  --agl
	{
	  'target_name': 'protobuf_full_do_not_use',
	  'type': 'static_library',
	  #'toolsets': ['host','target'],
	  'dependencies': [
			'../pthreads-win32/pthread.gyp:pthreads',
	  ],
	  'includes': [
		'protobuf_lite.gypi',
	  ],
	  'include_dirs': [
		'third_party/snappy/',
		'third_party/tinyxml/include/',
	  ],
	  'sources': [
		'src/google/protobuf/descriptor.h',
		'src/google/protobuf/descriptor.pb.h',
		'src/google/protobuf/descriptor_database.h',
		'src/google/protobuf/dynamic_message.h',
		'src/google/protobuf/generated_enum_reflection.h',
		'src/google/protobuf/generated_message_reflection.h',
		'src/google/protobuf/message.h',
		'src/google/protobuf/reflection_ops.h',
		'src/google/protobuf/service.h',
		'src/google/protobuf/text_format.h',
		'src/google/protobuf/wire_format.h',
		'src/google/protobuf/io/gzip_stream.h',
		'src/google/protobuf/io/printer.h',
		'src/google/protobuf/io/tokenizer.h',
		'src/google/protobuf/io/zero_copy_stream_impl.h',
		'src/google/protobuf/compiler/code_generator.h',
		'src/google/protobuf/compiler/command_line_interface.h',
		'src/google/protobuf/compiler/importer.h',
		'src/google/protobuf/compiler/java/java_doc_comment.cc',
		'src/google/protobuf/compiler/java/java_doc_comment.h',
		'src/google/protobuf/compiler/parser.h',

		'src/google/protobuf/stubs/strutil.cc',
		'src/google/protobuf/stubs/strutil.h',
		'src/google/protobuf/stubs/substitute.cc',
		'src/google/protobuf/stubs/substitute.h',
		'src/google/protobuf/stubs/stl_util.h',
		'src/google/protobuf/stubs/stringprintf.cc',
		'src/google/protobuf/stubs/stringprintf.h',
		'src/google/protobuf/stubs/structurally_valid.cc',
		'src/google/protobuf/stubs/template_util.h',
		'src/google/protobuf/stubs/type_traits.h',

		'src/google/protobuf/descriptor.cc',
		'src/google/protobuf/descriptor.pb.cc',
		'src/google/protobuf/descriptor_database.cc',
		'src/google/protobuf/dynamic_message.cc',
		'src/google/protobuf/extension_set.cc',
		'src/google/protobuf/extension_set.h',
		'src/google/protobuf/extension_set_heavy.cc',
		'src/google/protobuf/generated_message_reflection.cc',
		'src/google/protobuf/message.cc',
		'src/google/protobuf/reflection_ops.cc',
		'src/google/protobuf/service.cc',
		'src/google/protobuf/text_format.cc',
		'src/google/protobuf/wire_format.cc',
		# This file pulls in zlib, but it's not actually used by protoc, so
		# instead of compiling zlib for the host, let's just exclude this.
		# 'src/src/google/protobuf/io/gzip_stream.cc',
		'src/google/protobuf/io/printer.cc',
		'src/google/protobuf/io/tokenizer.cc',
		'src/google/protobuf/io/zero_copy_stream_impl.cc',
		'src/google/protobuf/compiler/importer.cc',
		'src/google/protobuf/compiler/parser.cc',
		
		# ThirdParty
		'third_party/snappy/snappy.h',
		'third_party/snappy/snappy-c.h',
		'third_party/snappy/snappy.cc',
		'third_party/snappy/snappy-c.cc',
		'third_party/snappy/snappy-sinksource.cc',
		'third_party/snappy/snappy-stubs-internal.cc',
		
		'third_party/tinyxml/include/tinyxml.h',
		'third_party/tinyxml/include/tinystr.h',
		'third_party/tinyxml/src/tinyxml.cpp',
		'third_party/tinyxml/src/tinystr.cpp',
		'third_party/tinyxml/src/tinyxmlerror.cpp',
		'third_party/tinyxml/src/tinyxmlparser.cpp',
		
		# RPC Implementation
		'src/google/protobuf/rpc/rpc_client.cc',
		'src/google/protobuf/rpc/rpc_client.h',
		'src/google/protobuf/rpc/rpc_conn.cc',
		'src/google/protobuf/rpc/rpc_conn.h',
		'src/google/protobuf/rpc/rpc_crc32.cc',
		'src/google/protobuf/rpc/rpc_crc32.h',
		'src/google/protobuf/rpc/rpc_env.cc',
		'src/google/protobuf/rpc/rpc_env.h',
		'src/google/protobuf/rpc/rpc_server.cc',
		'src/google/protobuf/rpc/rpc_server.h',
		'src/google/protobuf/rpc/rpc_server_conn.cc',
		'src/google/protobuf/rpc/rpc_server_conn.h',
		'src/google/protobuf/rpc/rpc_service.cc',
		'src/google/protobuf/rpc/rpc_service.h',
		'src/google/protobuf/rpc/rpc_wire.cc',
		'src/google/protobuf/rpc/rpc_wire.h',
		'src/google/protobuf/rpc/rpc_wire.h',
		'src/google/protobuf/rpc/wire.pb/wire.pb.cc',
		'src/google/protobuf/rpc/wire.pb/wire.pb.h',
		
		# XML Message
		'src/google/protobuf/xml/xml_base64.h',
		'src/google/protobuf/xml/xml_message.cc',
		'src/google/protobuf/xml/xml_message.h',
	  ],
	},
	{
	  'target_name': 'protoc',
	  'type': 'executable',
	  'sources': [
		'src/google/protobuf/compiler/code_generator.cc',
		'src/google/protobuf/compiler/command_line_interface.cc',
		'src/google/protobuf/compiler/plugin.cc',
		'src/google/protobuf/compiler/plugin.pb.cc',
		'src/google/protobuf/compiler/subprocess.cc',
		'src/google/protobuf/compiler/subprocess.h',
		'src/google/protobuf/compiler/zip_writer.cc',
		'src/google/protobuf/compiler/zip_writer.h',
		'src/google/protobuf/compiler/cxx25x/cxx_enum.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_enum.h',
		'src/google/protobuf/compiler/cxx25x/cxx_enum_field.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_enum_field.h',
		'src/google/protobuf/compiler/cxx25x/cxx_extension.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_extension.h',
		'src/google/protobuf/compiler/cxx25x/cxx_field.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_field.h',
		'src/google/protobuf/compiler/cxx25x/cxx_file.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_file.h',
		'src/google/protobuf/compiler/cxx25x/cxx_generator.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_generator.h',
		'src/google/protobuf/compiler/cxx25x/cxx_helpers.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_helpers.h',
		'src/google/protobuf/compiler/cxx25x/cxx_message.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_message.h',
		'src/google/protobuf/compiler/cxx25x/cxx_message_field.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_message_field.h',
		'src/google/protobuf/compiler/cxx25x/cxx_options.h',
		'src/google/protobuf/compiler/cxx25x/cxx_primitive_field.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_primitive_field.h',
		'src/google/protobuf/compiler/cxx25x/cxx_service.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_service.h',
		'src/google/protobuf/compiler/cxx25x/cxx_string_field.cc',
		'src/google/protobuf/compiler/cxx25x/cxx_string_field.h',
		'src/google/protobuf/compiler/cpp/cpp_enum.cc',
		'src/google/protobuf/compiler/cpp/cpp_enum.h',
		'src/google/protobuf/compiler/cpp/cpp_enum_field.cc',
		'src/google/protobuf/compiler/cpp/cpp_enum_field.h',
		'src/google/protobuf/compiler/cpp/cpp_extension.cc',
		'src/google/protobuf/compiler/cpp/cpp_extension.h',
		'src/google/protobuf/compiler/cpp/cpp_field.cc',
		'src/google/protobuf/compiler/cpp/cpp_field.h',
		'src/google/protobuf/compiler/cpp/cpp_file.cc',
		'src/google/protobuf/compiler/cpp/cpp_file.h',
		'src/google/protobuf/compiler/cpp/cpp_generator.cc',
		'src/google/protobuf/compiler/cpp/cpp_helpers.cc',
		'src/google/protobuf/compiler/cpp/cpp_helpers.h',
		'src/google/protobuf/compiler/cpp/cpp_message.cc',
		'src/google/protobuf/compiler/cpp/cpp_message.h',
		'src/google/protobuf/compiler/cpp/cpp_message_field.cc',
		'src/google/protobuf/compiler/cpp/cpp_message_field.h',
		'src/google/protobuf/compiler/cpp/cpp_primitive_field.cc',
		'src/google/protobuf/compiler/cpp/cpp_primitive_field.h',
		'src/google/protobuf/compiler/cpp/cpp_service.cc',
		'src/google/protobuf/compiler/cpp/cpp_service.h',
		'src/google/protobuf/compiler/cpp/cpp_string_field.cc',
		'src/google/protobuf/compiler/cpp/cpp_string_field.h',
		'src/google/protobuf/compiler/java/java_enum.cc',
		'src/google/protobuf/compiler/java/java_enum.h',
		'src/google/protobuf/compiler/java/java_enum_field.cc',
		'src/google/protobuf/compiler/java/java_enum_field.h',
		'src/google/protobuf/compiler/java/java_extension.cc',
		'src/google/protobuf/compiler/java/java_extension.h',
		'src/google/protobuf/compiler/java/java_field.cc',
		'src/google/protobuf/compiler/java/java_field.h',
		'src/google/protobuf/compiler/java/java_file.cc',
		'src/google/protobuf/compiler/java/java_file.h',
		'src/google/protobuf/compiler/java/java_generator.cc',
		'src/google/protobuf/compiler/java/java_helpers.cc',
		'src/google/protobuf/compiler/java/java_helpers.h',
		'src/google/protobuf/compiler/java/java_message.cc',
		'src/google/protobuf/compiler/java/java_message.h',
		'src/google/protobuf/compiler/java/java_message_field.cc',
		'src/google/protobuf/compiler/java/java_message_field.h',
		'src/google/protobuf/compiler/java/java_primitive_field.cc',
		'src/google/protobuf/compiler/java/java_primitive_field.h',
		'src/google/protobuf/compiler/java/java_service.cc',
		'src/google/protobuf/compiler/java/java_service.h',
		'src/google/protobuf/compiler/java/java_string_field.cc',
		'src/google/protobuf/compiler/java/java_string_field.h',
		'src/google/protobuf/compiler/python/python_generator.cc',
		'src/google/protobuf/compiler/main.cc',
	  ],
	  'dependencies': [
		'protobuf_full_do_not_use',
	  ],
	  'include_dirs': [
		'<(config_h_dir)',
		'src/src',
	  ],
	}
    ]
}
