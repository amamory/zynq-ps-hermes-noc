#
# build.tcl: Tcl script for re-creating project 'zedboard_base'
#
#*****************************************************************************************

# Check the version of Vivado used
set version_required "2018.2"
set vivado_dir $::env(XILINX_VIVADO)
if {[string first $version_required $vivado_dir] == -1} {
  puts "###############################"
  puts "### Failed to build project ###"
  puts "###############################"
  puts "This project was designed for use with Vivado $version_required."
  puts "Please install Vivado $version_required, or download the project"
  puts "sources from a commit of the Git repository that was intended for"
  puts "your version of Vivado."
  return
}

if { ![info exists env(VIVADO_DESIGN_NAME)] } {
    puts "Please set the environment variable VIVADO_DESIGN_NAME before running the script"
    return
}
set design_name $::env(VIVADO_DESIGN_NAME)
puts "Using design name: ${design_name}"

# Set the reference directory for source file relative paths (by default the value is script directory path)
set origin_dir "."

# Set the directory path for the original project from where this script was exported
set orig_proj_dir "[file normalize "$origin_dir/vivado/$design_name"]"

# Create project
create_project -force  $design_name $orig_proj_dir -part xc7z020clg484-1


# Set the directory path for the new project
set proj_dir [get_property directory [current_project]]

# Set project properties
set obj [get_projects $design_name]
set_property -name "board_part" -value "em.avnet.com:zed:part0:1.3" -objects $obj
set_property -name "default_lib" -value "xil_defaultlib" -objects $obj
set_property -name "ip_cache_permissions" -value "read write" -objects $obj
set_property -name "ip_output_repo" -value "$proj_dir/$design_name.cache/ip" -objects $obj
set_property -name "sim.ip.auto_export_scripts" -value "1" -objects $obj
set_property -name "simulator_language" -value "Mixed" -objects $obj

# set path to custom IPs
set_property  ip_repo_paths  ./vivado/ips [current_project]
update_ip_catalog


# Create 'sources_1' fileset (if not found)
if {[string equal [get_filesets -quiet sources_1] ""]} {
  create_fileset -srcset sources_1
}


# Set 'sources_1' fileset properties
set obj [get_filesets sources_1]
set_property "top" "${design_name}_wrapper" $obj

# TODO insert all the vhdl and verilog source files from ./hw/hdl into the project


# Create 'constrs_1' fileset (if not found)
if {[string equal [get_filesets -quiet constrs_1] ""]} {
  create_fileset -constrset constrs_1
}

# Set 'constrs_1' fileset object
set obj [get_filesets constrs_1]

# Add/Import constrs file and set constrs file properties
# TODO insert all XDC files from ./hw/xdc into the project
set constr_files [glob $origin_dir/hw/xdc/*.xdc]

#set file "[file normalize "$origin_dir/hw/xdc/const.xdc"]"
set file "[file normalize "$constr_files"]"
set file_added [add_files -norecurse -fileset $obj [list $file]]
set file_obj [get_files -of_objects [get_filesets constrs_1] [list "*$file"]]
set_property -name "file_type" -value "XDC" -objects $file_obj

# Set 'constrs_1' fileset properties
set obj [get_filesets constrs_1]
set_property -name "target_constrs_file" -value "[get_files $file]" -objects $obj
set_property -name "target_ucf" -value "[get_files $file]" -objects $obj

# Create 'sim_1' fileset (if not found)
if {[string equal [get_filesets -quiet sim_1] ""]} {
  create_fileset -simset sim_1
}

# Set 'sim_1' fileset object
set obj [get_filesets sim_1]

# Empty (no sources present)
# place here testbenches, waveform files, etc

# Set 'sim_1' fileset properties
set obj [get_filesets sim_1]
set_property "top" "${design_name}_wrapper" $obj

# Create 'synth_1' run (if not found)
if {[string equal [get_runs -quiet synth_1] ""]} {
  create_run -name synth_1 -part xc7z020clg484-1 -flow {Vivado Synthesis 2018} -strategy "Vivado Synthesis Defaults" -constrset constrs_1
} else {
  set_property strategy "Vivado Synthesis Defaults" [get_runs synth_1]
  set_property flow "Vivado Synthesis 2018" [get_runs synth_1]
}
set obj [get_runs synth_1]

# set the current synth run
current_run -synthesis [get_runs synth_1]

# Create 'impl_1' run (if not found)
if {[string equal [get_runs -quiet impl_1] ""]} {
  create_run -name impl_1 -part xc7z020clg484-1 -flow {Vivado Implementation 2018} -strategy "Vivado Implementation Defaults" -constrset constrs_1 -parent_run synth_1
} else {
  set_property strategy "Vivado Implementation Defaults" [get_runs impl_1]
  set_property flow "Vivado Implementation 2018" [get_runs impl_1]
}
set obj [get_runs impl_1]
set_property -name "steps.write_bitstream.args.readback_file" -value "0" -objects $obj
set_property -name "steps.write_bitstream.args.verbose" -value "0" -objects $obj

# set the current impl run
current_run -implementation [get_runs impl_1]

puts "INFO: Project created:${design_name}"

# Prepare to create a block design
# CHECKING IF PROJECT EXISTS
if { [get_projects -quiet] eq "" } {
   puts "ERROR: Please open or create a project!"
   return 1
}

# Create and empty block design
create_bd_design $design_name
current_bd_design $design_name

# Find the block tcl script and create the block design
set block_file [glob $origin_dir/hw/bd/*.tcl]
# TODO check if there is only one tcl file
source $block_file
create_root_design ""

# Generate the wrapper
make_wrapper -files [get_files *${design_name}.bd] -top
add_files -norecurse ./vivado/${design_name}/${design_name}.srcs/sources_1/bd/${design_name}/hdl/${design_name}_wrapper.v

# Update the compile order
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1

# Ensure parameter propagation has been performed
close_bd_design [current_bd_design]
open_bd_design [get_files ${design_name}.bd]
validate_bd_design -force
save_bd_design
