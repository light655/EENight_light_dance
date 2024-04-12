sudo openocd -f interface/cmsis-dap.cfg -c "adapter speed 5000" -f /target/rp2040.cfg -c "program $(dirname $0)/build/EENLD.elf verify reset exit"
