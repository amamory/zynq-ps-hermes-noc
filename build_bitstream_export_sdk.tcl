if { ![info exists env(VIVADO_DESIGN_NAME)] } {
    puts "Please set the environment variable VIVADO_DESIGN_NAME before running the script"
    return
}
set design_name $::env(VIVADO_DESIGN_NAME)
puts "Using design name: ${design_name}"

# generate bitstream
open_project ./vivado/${design_name}/${design_name}.xpr
update_compile_order -fileset sources_1
launch_runs impl_1 -to_step write_bitstream -jobs 8
wait_on_run impl_1

# exporting hw design to SDK
file mkdir ./vivado/${design_name}/${design_name}.sdk
file copy -force ./vivado/${design_name}/${design_name}.runs/impl_1/${design_name}_wrapper.sysdef ./vivado/${design_name}/${design_name}.sdk/${design_name}.hdf

#launch_sdk -workspace ./$design_name/zedboard_base.sdk -hwspec ./$design_name/zedboard_base.sdk/zedboard_base_wrapper.hdf

close_design
