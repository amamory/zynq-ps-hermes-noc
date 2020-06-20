# Zynq PS and Hermes

This repo contains scripts to recreate a Vivado and xSDK project Zynq PS connected to a [Hermes network-on-chip router](https://www.sciencedirect.com/science/article/abs/pii/S0167926004000185) via AXI streaming interface. The project is setup for Zedboard, although it would be easy to change to other boards assuming you have some basic TCL skills.

# How to run it

These scripts are assuming Linux operation system (Ubuntu 18.04) and Vivado 2018.2.

Follow these instructions to recreate the Vivado and SDK projects:
 - Open the **build.sh** script and edit the first two lines to setup the environment variables 
**VIVADO** and **VIVADO_DESIGN_NAME**, and **VIVADO_TOP_NAME** (optional). 
 - run *build.sh*

These scripts will recreate the entire Vivado project, compile the design, and generate the bitstream, export the hardware to SDK, create the SDK projects, import the source files, and build all projects. Hopefully, all the steps will be executed automatically.

# Future work

 - update the scripts to Vitis
 - make the script more generic, for example, board independent
 - support bitstream download and elf download
 - support or test with Windows (help required !!! :D )

# Credits

The scripts are based on the excelent scripts from [fpgadesigner](https://github.com/fpgadeveloper/zedboard-axi-dma) plus few increments from my own such as project generalization, support to SDK project creation and compilation and other minor improvements. 

