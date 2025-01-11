if ! test -d ./lib/FreeRTOS-Kernel; then
  echo "Cloning FreeRTOS-Kernel"
  git clone https://github.com/FreeRTOS/FreeRTOS-Kernel ./lib/FreeRTOS-Kernel
fi
