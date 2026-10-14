rm -rf */.pio SDCARD
mkdir SDCARD
cd ball_app       && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/BALL.BIN
cd ../bricks_app  && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/BRIX.BIN
cd ../chat_app    && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/CHAT.BIN
cd ../chess_app   && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/CHESS.BIN
cd ../code_app    && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/CODE.BIN
cd ../ccode_app   && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/CCODE.BIN
cd ../go_app      && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/GO.BIN
cd ../midigen_app && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/MIDI.BIN
cd ../morph_app   && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/MORPH.BIN
cd ../sensor_app  && pio run && cp .pio/build/esp32dev/firmware_mdos.bin ../SDCARD/SENS.BIN
cd ..
