dyncall_s = static_library('dyncall_s',
	sources : [
		'dyncall_call.S',
		'dyncall_vector.c',
		'dyncall_aggregate.c',
		'dyncall_api.c',
		'dyncall_callvm.c',
		'dyncall_callvm_base.c',
		'dyncall_callf.c',
	]
)

dyncall_s_dep = declare_dependency(
	link_with: dyncall_s,
	version: meson.project_version(),
	include_directories: [
		include_directories('.'),
	]
)
