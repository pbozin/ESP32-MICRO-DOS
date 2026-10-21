for board in "CYD_ST7796" "CYD_ST7789" "CYD_ILI9341" "JC3248"
do 
pio run -e ${board} --target clean && pio run -e ${board} && cp .pio/build/${board}/firmware.factory.bin test-ride_binaries/${board}.firmware.factory.bin
pio run -e ${board} --target clean
done && cd apps && ./build_all.sh && cp -r SDCARD/* ../test-ride_binaries/SDCARD/ && cd ..

