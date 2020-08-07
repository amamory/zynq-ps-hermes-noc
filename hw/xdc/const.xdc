# set the target packet header using the zedboard dip switches
set_property PACKAGE_PIN F22 [get_ports {dip_i[0]}]
set_property PACKAGE_PIN G22 [get_ports {dip_i[1]}]
set_property PACKAGE_PIN H22 [get_ports {dip_i[2]}]
set_property PACKAGE_PIN F21 [get_ports {dip_i[3]}]
set_property PACKAGE_PIN H19 [get_ports {dip_i[4]}]
set_property PACKAGE_PIN H18 [get_ports {dip_i[5]}]
set_property PACKAGE_PIN H17 [get_ports {dip_i[6]}]
set_property PACKAGE_PIN M15 [get_ports {dip_i[7]}]

# press the BTNC to end the packet transmission and return to the initial state
set_property PACKAGE_PIN P16 [get_ports end_i]

# select which source module will send the packet. 
# there are only two sources in this design, each one attached to a different router
# send_i[0] is attached to East port of R11
# send_i[1] is attached to West port of R01
set_property PACKAGE_PIN R18 [get_ports {send_i[0]}]
set_property PACKAGE_PIN N15 [get_ports {send_i[1]}]
#set_property PACKAGE_PIN T18 [get_ports {send_i[2]}]
#set_property PACKAGE_PIN R16 [get_ports {send_i[3]}]

# the receving port will turn the LED on
# LED [0 : 3] is attached to R01. 
#    The LEDs are connected to these ports: [0] stuck to 0, [1] W, [2] N, [3] S
# LED [4 : 7] is attached to R11. 
#    The LEDs are connected to these ports: [4] E, [5] N, [6] S, [7] L
set_property PACKAGE_PIN T22 [get_ports {led_o[0]}]
set_property PACKAGE_PIN T21 [get_ports {led_o[1]}]
set_property PACKAGE_PIN U22 [get_ports {led_o[2]}]
set_property PACKAGE_PIN U21 [get_ports {led_o[3]}]
set_property PACKAGE_PIN V22 [get_ports {led_o[4]}]
set_property PACKAGE_PIN W21 [get_ports {led_o[5]}]
set_property PACKAGE_PIN U19 [get_ports {led_o[6]}]
set_property PACKAGE_PIN U14 [get_ports {led_o[7]}]




set_property IOSTANDARD LVCMOS33 [get_ports {dip_i[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {dip_i[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {dip_i[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {dip_i[4]}]
set_property IOSTANDARD LVCMOS33 [get_ports {dip_i[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {dip_i[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {dip_i[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {dip_i[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports end_i]
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[4]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {send_i[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {send_i[1]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {send_i[2]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {send_i[3]}]

